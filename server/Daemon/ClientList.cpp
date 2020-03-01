#include<assert.h>
#include<errno.h>
#include<fstream>
#include<signal.h>
#include<sstream>
#include<string.h>
#include<unistd.h>
#include<unordered_set>
#include"Daemon/Breaker.hpp"
#include"Daemon/ClientList.hpp"
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

auto client_list_initialized = false;

auto pipe_write = int(-1);

void sighup_sig_handler(int) {
	auto const buffer = char(1);
	if (pipe_write >= 0) {
		auto res = write(pipe_write, &buffer, 1);
		if (res < 0)
			return;
	}
}

}

namespace Daemon {

class ClientList::Impl {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;

	std::unordered_set<Secp256k1::PubKey> clients;

	int pipe_read;

	void load_clients() {
		auto tmp_clients = std::unordered_set<Secp256k1::PubKey>();

		auto is = std::ifstream("clients");
		if (!is || is.bad() || !is.good()) {
			logger.unusual( "Could not open file 'clients'.  "
					"Not updating in-memory clients list."
				      );
			return;
		}

		while (is && !is.eof()) {
			auto s = std::string();
			is >> s;
			if (s == "")
				break;
			if (s.length() != 66 || (s[0] != 'c' && s[0] != 'c')) {
				logger.unusual("Invalid client id: %s"
					      , s.c_str()
					      );
				continue;
			}
			auto c0 = s[0];
			s[0] = '0';
			try {
				auto cid = Secp256k1::PubKey(s);
				tmp_clients.insert(cid);
			} catch(...) {
				s[0] = c0;
				logger.unusual( "Failed to parse client id: %s"
					      , s.c_str()
					      );
				continue;
			}
		}
		clients.swap(tmp_clients);
		for (auto& c : clients) {
			auto os = std::ostringstream();
			os << c;
			auto str = os.str();
			str[0] = 'c';
			logger.debug( "Client: %s"
				    , str.c_str()
				    );
		}
		if (clients.empty())
			logger.info("No clients. Create a `clients` file with client IDs.");
	}

	Ev::Io<int> sighup_loop() {
		assert(pipe_read >= 0);
		return breaker.wait_readable_or_break(pipe_read)
		     .then<int>([this](bool ok) {
			if (!ok)
				return Ev::lift_io(0);

			auto c = char();
			auto res = ssize_t();
			do {
				res = read(pipe_read, &c, 1);
			} while (res < 0 && errno == EINTR);
			logger.info("Received SIGHUP, re-loading `clients`.");
			load_clients();
			return sighup_loop();
		});
	}

public:
	Impl( Util::Logger& logger_
	    , Daemon::Breaker& breaker_
	    ) : logger(logger_), breaker(breaker_), pipe_read(-1) {
		if (client_list_initialized)
			throw std::logic_error("Multiple initializations of Daemon::ClientList");

		int fds[2];
		auto res = pipe(fds);
		if (res < 0) {
			auto msg = std::string(strerror(errno));
			logger.BROKEN( "Daemon::ClientList: pipe failed: %s"
				     , msg.c_str()
				     );
			throw std::logic_error( "Daemon::ClientList: "
						"pipe failed: "
					      + msg
					      );
		}

		struct sigaction sa;
		sa.sa_handler = &sighup_sig_handler;
		sigfillset(&sa.sa_mask);
		sa.sa_flags = 0;
		auto res_int = sigaction(SIGHUP, &sa, nullptr);
		if (res_int < 0) {
			auto msg = std::string(strerror(errno));
			logger.BROKEN( "Daemon::ClientList: sigaction(SIGHUP): %s"
				     , msg.c_str()
				     );
			close(fds[0]);
			close(fds[1]);
			throw std::logic_error( "Daemon::ClientList: "
						"sigaction(SIGHUP): "
					      + msg
					      );
		}

		pipe_read = fds[0];
		pipe_write = fds[1];

		load_clients();
	}

	~Impl() {
		signal(SIGHUP, SIG_DFL);
		close(pipe_write);
		close(pipe_read);
		pipe_write = -1;
	}

	Ev::Io<int> launch() {
		return Ev::lift_io(0).then<int>([this](int){
			logger.debug( "Monitoring SIGHUP on <fd %d> --- "
				      "raise SIGHUP to reload "
				      "`clients`."
				    , pipe_read
				    );
			return Ev::concurrent(sighup_loop());
		});
	}

	bool has(Secp256k1::PubKey const& pk) {
		return clients.find(pk) != clients.end();
	}
};

ClientList::ClientList( Util::Logger& logger
		      , Daemon::Breaker& breaker
		      ) : pimpl(Util::make_unique<Impl>(logger, breaker))
			{ }
ClientList::~ClientList() { }

std::unique_ptr<ClientList>
ClientList::initialize( Util::Logger& logger
		      , Daemon::Breaker& breaker
		      ) {
	/* Cannot use make_unique, constructor is private. */
	return std::unique_ptr<ClientList>(new ClientList(logger, breaker));
}

Ev::Io<int> ClientList::launch() {
	return pimpl->launch();
}

bool ClientList::has(Secp256k1::PubKey const& pk) {
	return pimpl->has(pk);
}

}

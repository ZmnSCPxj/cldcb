#include<assert.h>
#include<errno.h>
#include<fcntl.h>
#include<fstream>
#include<memory>
#include<signal.h>
#include<sstream>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<time.h>
#include<unistd.h>
#include"Net/Fd.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Server/change_clients.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

std::string gethostname_xx() {
	char c_hostname[256];
	(void) gethostname(c_hostname, sizeof(c_hostname));
	c_hostname[sizeof(c_hostname) - 1] = 0;
	return std::string(c_hostname);
}

/* Kickable RAII class to auto-remove the specific file.  */
class UnlinkFile {
private:
	std::unique_ptr<std::string> filename;

public:
	UnlinkFile() =delete;
	UnlinkFile(UnlinkFile&&) =default;

	explicit
	UnlinkFile( std::string filename_
		  ) : filename(Util::make_unique<std::string>(
			std::move(filename_)
		      ))
		    { }

	void do_not_unlink() {
		filename = nullptr;
	}

	~UnlinkFile() {
		if (filename)
			unlink(filename->c_str());
	}
};

/* Create a lockfile with the given path name.
 * If successful, returns the file descriptor
 * of the lockfile, which is open for writing.
 * We (ab)use Net::Fd to return a filedescriptor.
 */
Net::Fd
create_lockfile( std::string& error
	       , std::string const& lockfile_name
	       ) {
	auto p = getpid();
	auto t = time(nullptr);
	auto h = gethostname_xx();

	/* Truncate.  */
	t %= 16384;
	if (h.size() > 16)
		h.resize(16);
	/* FIXME: Probably better to concatenate them all
	 * together, then scramble in a short hash function.
	 */

	/* Generate per-process lockfile name.  */
	auto my_lockfile = ([p, t, &h]() {
		auto os = std::ostringstream();
		os << p << "." << t << "." << h;
		return os.str();
	})();

	/* Create our per-process lockfile.  */
	auto ret = Net::Fd();
	do {
		ret = Net::Fd(open( my_lockfile.c_str()
				  , O_WRONLY | O_CREAT | O_EXCL
				  , 0600
				  ));
	} while (!ret && errno == EINTR);
	if (!ret) {
		error = std::string("Failed to create lockfile: ")
		      + strerror(errno)
		      ;
		return ret;
	}
	auto unlinker = UnlinkFile(my_lockfile);

	/* Link.  */
	auto res_link = link(my_lockfile.c_str(), lockfile_name.c_str());
	(void) res_link;

	struct stat my_lockfile_stat;
	if (fstat(ret.get(), &my_lockfile_stat) < 0) {
		error = std::string("Failed to stat created lockfile: ")
		      + strerror(errno)
		      ;
		ret.reset(-1);
		return ret;
	}
	/* Did adding a new link succeed?  */
	if (my_lockfile_stat.st_nlink != 2) {
		error = "Failed to acquire lockfile";
		ret.reset(-1);
		return ret;
	}

	struct stat lockfile_name_stat;
	if (stat(lockfile_name.c_str(), &lockfile_name_stat) < 0) {
		error = std::string("Failed to stat linked lockfile: ")
		      + strerror(errno)
		      ;
		ret.reset(-1);
		return ret;
	}
	/* Did we ***really*** link to the target lockfile?  */
	if (my_lockfile_stat.st_ino != lockfile_name_stat.st_ino) {
		error = "Did not acquire lockfile";
		ret.reset(-1);
		return ret;
	}

	error = "";
	return ret;
}

Server::ClientSet
read_clients(Util::Logger& logger) {
	auto clients = Server::ClientSet();
	auto is = std::ifstream("clients");
	if (!is || is.bad() || !is.good()) {
		logger.debug("clients file could not be opened.");
		return clients;
	}

	while (is && !is.eof()) {
		auto s = std::string();
		is >> s;
		if (s == "")
			break;
		logger.debug("client: %s", s.c_str());
		if (s.length() != 66 || (s[0] != 'c' && s[0] != 'C'))
			/* Ignore.  */
			continue;
		s[0] = '0';
		auto cid = Secp256k1::PubKey(s);
		clients.insert(cid);
	}
	return clients;
}

auto constexpr max_tries = 5;

pid_t get_server_pid() {
	auto is = std::ifstream("cldcb-server.pid");
	if (!is || is.bad() || !is.good())
		return (pid_t)-1;
	auto ret = pid_t();
	is >> ret;
	return ret;
}

}

namespace Server {

std::string
change_clients( Util::Logger& logger
	      , std::function<void(ClientSet&)> changer
	      ) {
	auto error = std::string("");
	auto to_write = Net::Fd();
	for (auto tries = 0; tries < max_tries; ++tries) {
		to_write = create_lockfile(error, "clients.lock");
		if (to_write)
			break;
		if (tries == max_tries - 1) {
			logger.BROKEN( "Could not acquire clients.lock, "
				       "aborting: %s"
				     , error.c_str()
				     );
			return error;
		}
		logger.info("Could not acquire clients.lock, will sleep: %s"
			   , error.c_str()
			   );
		sleep(2);
	}
	auto unlinker = UnlinkFile("clients.lock");

	assert(to_write);

	auto clients = read_clients(logger);
	changer(clients);
	for (auto& client : clients) {
		auto client_string = ([&client]() {
			auto os = std::ostringstream();
			os << client << std::endl;
			auto s = os.str();
			s[0] = 'c';
			return s;
		})();

		auto p = &client_string[0];
		auto s = client_string.length();
		do {
			auto res = ssize_t();
			do {
				res = write(to_write.get(), p, s);
			} while (res < 0 && errno == EINTR);
			if (res < 0) {
				error = std::string("Failed to write "
						    "new clients file: "
						   )
				      + strerror(errno)
				      ;
				return error;
			}
			p += res;
			s -= std::size_t(res);
		} while (s > 0);
	}

	/* Atomically remove lock and replace with new clients list.  */
	auto res = rename("clients.lock", "clients");
	if (res < 0) {
		error = std::string("Failed to replace clients file: ")
		      + strerror(errno)
		      ;
		return error;
	}
	unlinker.do_not_unlink();

	auto server_pid = get_server_pid();
	if (server_pid > 0 && kill(server_pid, SIGHUP) == 0)
		logger.info("Triggered reload at server.");

	error = "";
	return error;
}

}

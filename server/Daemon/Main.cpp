#include<assert.h>
#include"Daemon/AcceptHandler.hpp"
#include"Daemon/AcceptLoop.hpp"
#include"Daemon/Breaker.hpp"
#include"Daemon/ClientList.hpp"
#include"Daemon/KeyKeeper.hpp"
#include"Daemon/Main.hpp"
#include"Daemon/PidFiler.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

class Main::Impl {
private:
	/* This is in order of construction!  */
	Daemon::PidFiler pidfiler;
	Daemon::KeyKeeper keeper;
	std::unique_ptr<Daemon::Breaker> breaker;
	std::unique_ptr<Daemon::ClientList> clients;
	Daemon::AcceptHandler accept_handler;
	Daemon::AcceptLoop acceptor;

public:
	Impl() =delete;
	Impl( Util::Logger& logger
	    , int port
	    , std::string pid_path
	    ) : pidfiler(logger, std::move(pid_path))
	      , keeper(logger)
	      , breaker(Daemon::Breaker::initialize(logger))
	      , clients(Daemon::ClientList::initialize(logger, *breaker))
	      , accept_handler( logger
			      , *breaker
			      , keeper.get_server_keypair()
			      , "CLDCB"
			      )
	      , acceptor(port, logger, *breaker, accept_handler)
	      { }

	void run() {
		(void) Ev::start(clients->launch().then<int>([this](int) {
			return acceptor.accept_loop();
		}));
	}
};

Main::Main( Util::Logger& logger
	  , int port
	  , std::string pid_path
	  )
	: pimpl(Util::make_unique<Impl>(logger, port, std::move(pid_path)))
	{ }

Main::~Main() { }

void Main::run() {
	assert(pimpl);
	pimpl->run();
}

}

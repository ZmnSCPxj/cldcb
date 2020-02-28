#include<assert.h>
#include"Daemon/AcceptHandler.hpp"
#include"Daemon/AcceptLoop.hpp"
#include"Daemon/Breaker.hpp"
#include"Daemon/Main.hpp"
#include"Daemon/PidFiler.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

class Main::Impl {
private:
	Daemon::PidFiler pidfiler;
	std::unique_ptr<Daemon::Breaker> breaker;
	Daemon::AcceptHandler accept_handler;
	Daemon::AcceptLoop acceptor;

public:
	Impl() =delete;
	Impl( Util::Logger& logger
	    , int port
	    , std::string pid_path
	    ) : pidfiler(logger, std::move(pid_path))
	      , breaker(Daemon::Breaker::initialize(logger))
	      , accept_handler(logger, *breaker)
	      , acceptor(port, logger, *breaker, accept_handler)
	      { }

	void run() {
		(void) Ev::start(acceptor.accept_loop());
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

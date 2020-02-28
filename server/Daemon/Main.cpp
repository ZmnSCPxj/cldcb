#include<assert.h>
#include"Daemon/AcceptHandler.hpp"
#include"Daemon/AcceptLoop.hpp"
#include"Daemon/Breaker.hpp"
#include"Daemon/Main.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

class Main::Impl {
private:
	std::unique_ptr<Daemon::Breaker> breaker;
	Daemon::AcceptHandler accept_handler;
	Daemon::AcceptLoop acceptor;

public:
	Impl() =delete;
	Impl( Util::Logger& logger
	    , int port
	    ) : breaker(Daemon::Breaker::initialize(logger))
	      , accept_handler(logger, *breaker)
	      , acceptor(port, logger, *breaker, accept_handler)
	      { }

	void run() {
		(void) Ev::start(acceptor.accept_loop());
	}
};

Main::Main( Util::Logger& logger
	  , int port
	  )
	: pimpl(Util::make_unique<Impl>(logger, port))
	{ }

Main::~Main() { }

void Main::run() {
	assert(pimpl);
	pimpl->run();
}

}

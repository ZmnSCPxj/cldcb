#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/yield.hpp"
#include"Daemon/AcceptLoop.hpp"
#include"Net/Listener.hpp"
#include"Net/SocketFd.hpp"
#include"Net/make_nonblocking.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

class AcceptLoop::Impl {
private:
	Net::Listener listener;
	Util::Logger& logger;
	Daemon::Breaker& breaker;
	std::function<Ev::Io<int>(Net::SocketFd)> handler;


public:
	Impl() =delete;
	Impl( int port
	    , Util::Logger& logger_
	    , Daemon::Breaker& breaker_
	    , std::function<Ev::Io<int>(Net::SocketFd)> handler_
	    ) : listener(port, logger_)
	      , logger(logger_)
	      , breaker(breaker_)
	      , handler(std::move(handler_))
	      {
		/* Make listening socket nonblocking.  */
		Net::make_nonblocking(listener.get_fd());
	}

	Ev::Io<int> accept_loop() {
		return Ev::yield().then<bool>([this](int) {
			return breaker.wait_readable_or_break(listener.get_fd());
		}).then<int>([this](bool ok) {
			if (!ok) {
				logger.debug("Break received, "
					     "leaving listener <fd %d>."
					    , listener.get_fd()
					    );
				return Ev::lift_io(0);
			} else {
				auto socket_fd = listener.accept();
				if (!socket_fd) {
					logger.debug( "accept failed: "
						      "%s"
						    , strerror(errno)
						    );
					return accept_loop();
				} else {
					Net::make_nonblocking(socket_fd.get());
					auto forked = handler(std::move(socket_fd));
					return Ev::concurrent(forked)
					     .then<int>([this](int) {
						return accept_loop();
					});
				}
			}
		});
	}
};

AcceptLoop::AcceptLoop( int port
		      , Util::Logger& logger
		      , Daemon::Breaker& breaker
		      , std::function<Ev::Io<int>(Net::SocketFd)> handler
		      ) : pimpl(Util::make_unique<Impl>( port
						       , logger
						       , breaker
						       , std::move(handler)
						       ))
			{ }

AcceptLoop::~AcceptLoop() { }

Ev::Io<int> AcceptLoop::accept_loop() {
	return pimpl->accept_loop();
}

}

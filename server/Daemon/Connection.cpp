#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Daemon/Connection.hpp"
#include"Ev/Io.hpp"
#include"Util/Logger.hpp"

namespace Daemon {

Connection::Connection( Util::Logger& logger_
		      , Daemon::Breaker& breaker_
		      , Net::SocketFd fd_
		      ) : logger(logger_)
			, breaker(breaker_)
			, fd(std::move(fd_))
			{ }

Ev::Io<int> Connection::new_connection(std::shared_ptr<Connection> self) {
	/* TODO.  */
	return self->breaker.wait_writeable_or_break(self->fd.get())
	     . then<int>([self](bool ok) {
		if (ok) {
			auto res = write( self->fd.get()
					, "Hello World.\n"
					, sizeof("Hello World.")
					);
			return Ev::lift_io<int>(int(res));
		} else
			return Ev::lift_io<int>(0);
	});
}

}

#ifndef CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP
#define CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Net { class SocketFd; }
namespace Util { class Logger; }

namespace Daemon {

/* Handles a newly-accepted connection.
 * Install this into the constructor of Daemon::AcceptLoop
 * in order to set up a connection at each accept.
 */
class AcceptHandler {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;

public:
	AcceptHandler( Util::Logger& logger_
		     , Daemon::Breaker& breaker_
		     ) : logger(logger_), breaker(breaker_) { }

	Ev::Io<int> operator()(Net::SocketFd);
};

}

#endif /* CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP */

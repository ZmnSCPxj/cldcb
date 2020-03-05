#ifndef CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP
#define CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP

#include<string>

namespace Daemon { class Breaker; }
namespace Daemon { class ConnectionLoop; }
namespace Ev { template<typename a> class Io; }
namespace Net { class SocketFd; }
namespace Secp256k1 { class KeyPair; }
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
	Daemon::ConnectionLoop& looper;
	Secp256k1::KeyPair const& identity;
	std::string prologue;

public:
	AcceptHandler( Util::Logger& logger_
		     , Daemon::Breaker& breaker_
		     , Daemon::ConnectionLoop& looper_
		     , Secp256k1::KeyPair const& identity_
		     , std::string const& prologue_ = "lightning"
		     ) : logger(logger_)
		       , breaker(breaker_)
		       , looper(looper_)
		       , identity(identity_)
		       , prologue(prologue_)
		       { }

	Ev::Io<int> operator()(Net::SocketFd);
};

}

#endif /* CLDCB_SERVER_DAEMON_ACCEPTHANDLER_HPP */

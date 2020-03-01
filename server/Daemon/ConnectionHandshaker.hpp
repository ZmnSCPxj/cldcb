#ifndef CLDCB_SERVER_DAEMON_CONNECTIONHANDSHAKER_HPP
#define CLDCB_SERVER_DAEMON_CONNECTIONHANDSHAKER_HPP

#include<memory>
#include"Noise/Responder.hpp"

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Net { class SocketFd; }
namespace Noise { class Encryptor; }
namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace Daemon {

class ConnectionHandshaker {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;
	Noise::Responder responder;
	Net::SocketFd const& fd;

public:
	ConnectionHandshaker() =delete;
	/* We only keep references to the logger, breaker,
	 * and fd.
	 * Client code is responsible for keeping those
	 * objects alive.
	 */
	ConnectionHandshaker( Util::Logger& logger
			    , Daemon::Breaker& breaker
			    , Secp256k1::KeyPair const& identity
			    , std::string const& prologue
			    , Net::SocketFd const& fd
			    );

	Ev::Io<std::unique_ptr<std::pair<Noise::Encryptor, Secp256k1::PubKey>>>
	handshake();
};

}

#endif /* CLDCB_SERVER_DAEMON_CONNECTIONHANDSHAKER_HPP */

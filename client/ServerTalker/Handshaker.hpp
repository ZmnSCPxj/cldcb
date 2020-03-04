#ifndef CLDCB_CLIENT_SERVERTALKER_HANDSHAKER_HPP
#define CLDCB_CLIENT_SERVERTALKER_HANDSHAKER_HPP

#include<memory>
#include"Noise/Initiator.hpp"

namespace Net { class SocketFd; }
namespace Noise { class Encryptor; }
namespace Secp256k1 { class KeyPair; }
namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace ServerTalker {

/* Performs handshaking with the server.  */
class Handshaker {
private:
	Util::Logger& logger;
	Noise::Initiator initiator;
	Net::SocketFd const& fd;

public:
	Handshaker() =delete;
	Handshaker(Handshaker&&) =delete;
	Handshaker(Handshaker const&) =delete;

	Handshaker( Util::Logger& logger
		  , Secp256k1::KeyPair const& identity
		  , Secp256k1::PubKey const& server_identity
		  , std::string const& prologue
		  , Net::SocketFd const& fd
		  );

	/* Return nullptr on failure.  */
	std::unique_ptr<Noise::Encryptor> handshake();
};

}

#endif /* CLDCB_CLIENT_SERVERTALKER_HANDSHAKER_HPP */

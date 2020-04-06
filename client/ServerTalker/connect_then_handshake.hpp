#ifndef CLDCB_CLIENT_SERVERTALKER_CONNECT_THEN_HANDSHAKE_HPP
#define CLDCB_CLIENT_SERVERTALKER_CONNECT_THEN_HANDSHAKE_HPP

#include<memory>
#include<string>

namespace Net { class Connector; }
namespace Secp256k1 { class KeyPair; }
namespace Secp256k1 { class PubKey; }
namespace ServerTalker { class MessengerIf; }
namespace Util { class Logger; }

namespace ServerTalker {

/* Connects to the server and performs the initial handshake,
 * then returns a messenger that can be used to send and
 * receive from the server.
 */
std::unique_ptr<ServerTalker::MessengerIf>
connect_then_handshake( Util::Logger& logger
		      , Net::Connector& connector
		      , Secp256k1::KeyPair const& client_id
		      , Secp256k1::PubKey const& server_id
		      , std::string const& server_host
		      , int server_port
		      );

}

#endif /* CLDCB_CLIENT_SERVERTALKER_CONNECT_THEN_HANDSHAKE_HPP */

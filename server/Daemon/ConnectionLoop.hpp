#ifndef CLDCB_SERVER_DAEMON_CONNECTIONLOOP_HPP
#define CLDCB_SERVER_DAEMON_CONNECTIONLOOP_HPP

#include<functional>

namespace Ev { template<typename a> class Io; }
namespace Net { class SocketFd; }
namespace Noise { class Encryptor; }
namespace Secp256k1 { class PubKey; }

namespace Daemon {

/* An abstract class which, when given a new incoming
 * handshaked connection, returns a function that will
 * loop forever handling that connection (or at least
 * until a break / disconnection) afterwards.
 */
class ConnectionLoop {
public:
	virtual ~ConnectionLoop() { }

	virtual
	std::function<Ev::Io<int>()>
	new_handshaked_connection( Net::SocketFd
				 , Noise::Encryptor
				 , Secp256k1::PubKey const&
				 ) =0;
};


}

#endif /* CLDCB_SERVER_DAEMON_CONNECTIONLOOP_HPP */

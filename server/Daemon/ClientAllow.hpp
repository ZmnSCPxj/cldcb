#ifndef CLDCB_SERVER_DAEMON_CLIENTALLOW_HPP
#define CLDCB_SERVER_DAEMON_CLIENTALLOW_HPP

namespace Secp256k1 { class PubKey; }

namespace Daemon {

/* Abstarct interface to an object that determines if a client is
 * allowed or not allowed to write to the server.
 */
class ClientAllow {
public:
	virtual ~ClientAllow() { }

	virtual
	bool has(Secp256k1::PubKey const&) const =0;
};

}

#endif /* CLDCB_SERVER_DAEMON_CLIENTALLOW_HPP */

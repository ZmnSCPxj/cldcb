#ifndef CLDCB_SERVER_DAEMON_KEYKEEPER_HPP
#define CLDCB_SERVER_DAEMON_KEYKEEPER_HPP

#include"Secp256k1/KeyPair.hpp"

namespace Util { class Logger; }

namespace Daemon {

class KeyKeeper {
private:
	Secp256k1::KeyPair k;

public:
	KeyKeeper( Util::Logger& logger );

	Secp256k1::PrivKey const& get_server_pk() const { return k.priv(); }
	Secp256k1::PubKey const& get_server_id() const { return k.pub(); }
	Secp256k1::KeyPair const& get_server_keypair() const { return k; }
};

}

#endif /* CLDCB_SERVER_DAEMON_KEYKEEPER_HPP */

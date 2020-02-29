#ifndef CLDCB_SERVER_DAEMON_KEYKEEPER_HPP
#define CLDCB_SERVER_DAEMON_KEYKEEPER_HPP

#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Util { class Logger; }

namespace Daemon {

class KeyKeeper {
private:
	Secp256k1::PrivKey server_pk;
	Secp256k1::PubKey server_id;

public:
	KeyKeeper( Util::Logger& logger );

	Secp256k1::PrivKey const& get_server_pk() const { return server_pk; }
	Secp256k1::PubKey const& get_server_id() const { return server_id; }
};

}

#endif /* CLDCB_SERVER_DAEMON_KEYKEEPER_HPP */

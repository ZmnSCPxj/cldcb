#ifndef CLDCB_CLIENT_PLUGIN_SERVERSPEC_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERSPEC_HPP

#include<string>
#include"Secp256k1/PubKey.hpp"

namespace Plugin {

struct ServerSpec {
	Secp256k1::PubKey id;
	std::string host;
	int port;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERSPEC_HPP */

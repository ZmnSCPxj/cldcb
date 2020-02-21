#ifndef CLDCB_CLIENT_PLUGIN_SETUP_HPP
#define CLDCB_CLIENT_PLUGIN_SETUP_HPP

#include<string>
#include<vector>
#include"Plugin/ServerSpec.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Plugin {

struct Setup {
	/* PubKey of the node.  */
	Secp256k1::PubKey node_id;
	/* TODO: signature.  */

	/* Plugin pubkey.  */
	Secp256k1::PubKey our_id;
	/* Plugin private key.  FIXME: Put in a higher-security arena.  */
	Secp256k1::PrivKey our_priv_key;

	/* Proxy specifications.  */
	bool has_proxy;
	std::string proxy_host;
	int proxy_port;

	/* Servers to back up to.  */
	std::vector<Plugin::ServerSpec> servers;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SETUP_HPP */

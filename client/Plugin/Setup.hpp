#ifndef CLDCB_CLIENT_PLUGIN_SETUP_HPP
#define CLDCB_CLIENT_PLUGIN_SETUP_HPP

#include<string>
#include<vector>
#include"Plugin/ServerSpec.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Signature.hpp"

namespace Plugin {

class Setup {
public:
	/* PubKey of the node.  */
	Secp256k1::PubKey node_id;
	/* Signature of the node.
	 * The message being signed is the string
	 * "DO NOT SIGN THIS UNLESS YOU WANT YOUR FUNDS TO BE STOLEN "
	 * followed by the lowercase hex dump of the ECDH between
	 * the client and node keys.
	 * We need to sign a shared secret so that a third party
	 * cannot extract the public key of the node, which would be
	 * possible to get from a signature if the message is known.
	 * By making the message itself a secret, we prevent third
	 * party servers from learning the node id that is using a
	 * particular client id.
	 */
	Secp256k1::Signature node_sig;

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

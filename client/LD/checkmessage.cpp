#include"LD/checkmessage.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Signature.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace LD {

bool checkmessage( Secp256k1::PubKey const& pk
		 , Secp256k1::Signature const& sig
		 , std::string const& message
		 ) {
	/* Spec: https://twitter.com/rusty_twit/status/1182102005914800128  */
	/* Prepend the standard string.  */
	auto full_message = "Lightning Signed Message:" + message;
	/* Hash once.  */
	auto h1 = Sha256::fun(full_message.c_str(), full_message.length());
	/* Hash twice.  */
	std::uint8_t h1_buff[32];
	h1.to_buffer(h1_buff);
	auto h2 = Sha256::fun(h1_buff, 32);

	return sig.valid(pk, h2);
}

}

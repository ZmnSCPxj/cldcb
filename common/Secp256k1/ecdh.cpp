#include<sodium/utils.h>
#include"Crypto/Secret.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Sha256/fun.hpp"
#include"Sha256/Hash.hpp"

namespace Secp256k1 {

Crypto::Secret ecdh(PrivKey a, PubKey B) {
	std::uint8_t point_buffer[33];
	(a * B).to_buffer(point_buffer);

	std::uint8_t hash_buffer[32];
	Sha256::fun(point_buffer, sizeof(point_buffer)).to_buffer(hash_buffer);

	auto ret = Crypto::Secret::from_buffer(hash_buffer);

	sodium_memzero(point_buffer, sizeof(point_buffer));
	sodium_memzero(hash_buffer, sizeof(hash_buffer));

	/* We depend on C++ return value optimization to
	 * ensure that the returned Crypto::Secret is placed
	 * wherever the caller wants it placed.
	 * Note that if you want to place the secret in a secure
	 * memory somehow, you have to *construct* the secret
	 * there, not copy-assign the result of this function to
	 * an already-constructed secret.
	 * i.e. doing "secret = Secp256k1::ecdh(a, B)" results
	 * in a temporary object being constructed on the stack,
	 * then copied into the variable.
	 */

	return ret;
}

}


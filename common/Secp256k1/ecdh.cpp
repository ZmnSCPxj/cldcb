#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Sha256/fun.hpp"
#include"Sha256/Hash.hpp"

namespace Secp256k1 {

PrivKey ecdh(PrivKey a, PubKey B) {
	std::uint8_t point_buffer[33];
	(a * B).to_buffer(point_buffer);

	std::uint8_t hash_buffer[32];
	Sha256::fun(point_buffer, sizeof(point_buffer)).to_buffer(hash_buffer);

	return Secp256k1::PrivKey::from_buffer(hash_buffer);
}

}


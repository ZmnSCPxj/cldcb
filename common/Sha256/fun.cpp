#include<sha-256.h>
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Sha256 {

Hash fun(void const *p, std::size_t len) {
	std::uint8_t hash[32];
	calc_sha_256(hash, p, len);
	return Hash::from_buffer(hash);
}

}

#include<sodium/crypto_hash_sha256.h>
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Sha256 {

Hash fun(void const *p, std::size_t len) {
	std::uint8_t hash[32];
	crypto_hash_sha256( hash
			  , reinterpret_cast<unsigned char const*>(p)
			  , len
			  );
	return Hash::from_buffer(hash);
}

}

#include<cstdint>
#include<cstring>
#include<vector>
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace {

constexpr std::size_t block_size = 64;

}

namespace Sha256 {

Hash hmac( void const *key, std::size_t keylen
	 , void const *data, std::size_t datalen
	 ) {
	std::uint8_t fin_key[block_size];
	std::memset(fin_key, 0, block_size);

	if (keylen > block_size) {
		auto h = Sha256::fun(key, keylen);
		h.to_buffer(fin_key);
	} else {
		std::memcpy(fin_key, key, keylen);
	}

	auto fin_data = std::vector<std::uint8_t>(block_size + datalen);
	/* Input padding.  */
	std::memcpy(&fin_data[0], fin_key, block_size);
	for (auto i = 0; i < block_size; ++i)
		fin_data[i] ^= 0x36;
	/* Data.  */
	std::memcpy(&fin_data[block_size], data, datalen);

	/* Inner hash.  */
	auto hash1 = Sha256::fun(&fin_data[0], block_size + datalen);

	auto stage2 = std::vector<std::uint8_t>(block_size + sizeof(Hash));
	/* Output padding.  */
	std::memcpy(&stage2[0], fin_key, block_size);
	for (auto i = 0; i < block_size; ++i)
		stage2[i] ^= 0x5c;
	/* Inner hash.  */
	hash1.to_buffer(&stage2[block_size]);

	/* Outer hash.  */
	return Sha256::fun(&stage2[0], block_size + sizeof(Hash));
}

}

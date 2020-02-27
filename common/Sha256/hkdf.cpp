#include<assert.h>
#include<cstdint>
#include<vector>
#include"Sha256/Hash.hpp"
#include"Sha256/hkdf.hpp"
#include"Sha256/hmac.hpp"

namespace Sha256 {

void hkdf( void* vokm, std::size_t okmlen
	 , void const* salt, std::size_t saltlen
	 , void const* ikm, std::size_t ikmlen
	 ) {
	auto okm = reinterpret_cast<std::uint8_t*>(vokm);

	auto prk_hash = hmac(salt, saltlen, ikm, ikmlen);
	std::uint8_t prk[32];
	prk_hash.to_buffer(prk);

	auto n = (okmlen + 31) / 32; /* ceil(l / hashlen) */
	assert(n < 256);
	auto t = std::vector<std::uint8_t>();
	auto inp = std::vector<std::uint8_t>();
	auto okm_i = std::size_t(0);
	for (auto i = std::size_t(0); i < n; ++i) {
		inp = std::move(t);
		inp.push_back(std::uint8_t(i + 1));
		auto t_hash = hmac( prk, sizeof(prk)
				  , &inp[0], inp.size()
				  );
		t.resize(32);
		t_hash.to_buffer(&t[0]);

		for (auto j = 0; j < 32 && okm_i < okmlen; ++j, ++okm_i) {
			okm[okm_i] = t[j];
		}
	}
}

}

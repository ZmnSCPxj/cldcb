#include"Crypto/Secret.hpp"
#include"Noise/Detail/hkdf2.hpp"
#include"Sha256/hkdf.hpp"

namespace Noise { namespace Detail {

std::pair<Crypto::Secret, Crypto::Secret>
hkdf2(Crypto::Secret const& salt, Crypto::Secret const& ikm) {
	auto ret = std::make_pair(Crypto::Secret(), Crypto::Secret());

	std::uint8_t buffer[64];

	Sha256::hkdf( buffer, sizeof(buffer)
		    , salt.get_buffer(), 32
		    , ikm.get_buffer(), 32
		    );

	ret.first = Crypto::Secret::from_buffer(&buffer[0]);
	ret.second = Crypto::Secret::from_buffer(&buffer[32]);

	return ret;
}

std::pair<Crypto::Secret, Crypto::Secret>
hkdf2_zero(Crypto::Secret const& salt) {
	auto ret = std::make_pair(Crypto::Secret(), Crypto::Secret());

	std::uint8_t buffer[64];

	Sha256::hkdf( buffer, sizeof(buffer)
		    , salt.get_buffer(), 32
		    , NULL, 0
		    );

	ret.first = Crypto::Secret::from_buffer(&buffer[0]);
	ret.second = Crypto::Secret::from_buffer(&buffer[32]);

	return ret;
}

}}

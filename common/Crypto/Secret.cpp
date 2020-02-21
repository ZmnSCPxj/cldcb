#include<sodium/utils.h>
#include"Crypto/Secret.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Str.hpp"

namespace Crypto {

Secret::Secret(std::string const& s) {
	auto data = Util::Str::hexread(s);
	if (data.size() != 32)
		throw Util::Str::HexParseFailure("Crypto::Secret requires 32 bytes (64 hex digits)");

	for (auto i = 0; i < 32; ++i)
		secret[i] = data[i];
}

Secret::Secret(Secp256k1::Random& rand) {
	for (auto i = 0; i < 32; ++i)
		secret[i] = rand.get();
}

Secret::~Secret() {
	sodium_memzero(secret, sizeof(secret));
}

bool Secret::operator==(Secret const& o) const {
	return sodium_memcmp(secret, o.secret, 32) == 0;
}

std::ostream& operator<<(std::ostream& os, Secret const& s) {
	std::uint8_t buffer[32];
	s.to_buffer(buffer);
	for (auto i = 0; i < 32; ++i)
		os << Util::Str::hexbyte(buffer[i]);
	return os;
}

}

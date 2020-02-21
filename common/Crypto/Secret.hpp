#ifndef CLDCB_COMMON_CRYPTO_SECRET_HPP
#define CLDCB_COMMON_CRYPTO_SECRET_HPP

#include<cstdint>
#include<ostream>

namespace Secp256k1 { class Random; }

namespace Crypto {

/* A 256-bit secret without restrictions.
 * This is a separate class from Secp256k1::PrivKey
 * since a truly random 256-bit number has a 1 in
 * 2^128 chance of not being a valid SECP256K1 scalar.
 */
class Secret {
private:
	std::uint8_t secret[32];

public:
	Secret() {
		for (auto i = 0; i < 32; ++i)
			secret[i] = 0;
	}
	explicit Secret(std::string const&);
	explicit Secret(Secp256k1::Random&);
	Secret(Secret const& o) {
		for (auto i = 0; i < 32; ++i)
			secret[i] = o.secret[i];
	}
	Secret(Secret&&) =default;

	~Secret();

	Secret& operator=(Secret const&) =default;
	Secret& operator=(Secret&&) =default;

	bool operator==(Secret const&) const;
	bool operator!=(Secret const& o) const {
		return !(*this == o);
	}

	static Secret from_buffer(std::uint8_t const buffer[32]) {
		auto ret = Secret();
		for (auto i = 0; i < 32; ++i)
			ret.secret[i] = buffer[i];
		return ret;
	}
	void to_buffer(std::uint8_t buffer[32]) const {
		for (auto i = 0; i < 32; ++i)
			buffer[i] = secret[i];
	}

	/* TODO: Or consider making relevant algorithms
	 * friends of this class.
	 */
	std::uint8_t const* get_buffer() const {
		return secret;
	}
};

std::ostream& operator<<(std::ostream&, Secret const&);

}

#endif /* CLDCB_COMMON_CRYPTO_SECRET_HPP */

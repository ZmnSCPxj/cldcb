#ifndef CLDCB_COMMON_SECP256K1_PRIVKEY_HPP
#define CLDCB_COMMON_SECP256K1_PRIVKEY_HPP

#include<cstdint>
#include<ostream>
#include<stdexcept>
#include<string>
#include"S.hpp"

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Secp256k1 { class Random; }
namespace Secp256k1 { class Signature; }

std::ostream& operator<<(std::ostream&, Secp256k1::PrivKey const&);

namespace Secp256k1 {

/* Thrown if caller-provided data would result in an invalid
 * private key.
 */
/* FIXME: derive from a backtrace-capturing exception.  */
class InvalidPrivKey : public std::invalid_argument {
public:
	InvalidPrivKey() : std::invalid_argument("Invalid private key.") {}
};

class PrivKey {
private:
	std::uint8_t key[32];

	PrivKey( std::uint8_t const key_[32] );

public:
	PrivKey();

	/* Load private key from a hex-encoded string.  */
	explicit PrivKey(std::string const&);
	/* Pick a random private key.  */
	explicit PrivKey(Secp256k1::Random& rand);
	/* Copy an existing private key.  */
	PrivKey(PrivKey const&);

	~PrivKey();

	PrivKey& negate();
	PrivKey operator-() const {
		auto tmp = *this;
		tmp.negate();
		return tmp;
	}
	PrivKey& operator +=(PrivKey const&);
	PrivKey& operator -=(PrivKey const& o) {
		return (*this += -o);
	}
	PrivKey& operator *=(PrivKey const& o);

	PrivKey operator+(PrivKey const& o) const {
		auto tmp = *this;
		tmp += o;
		return tmp;
	}
	PrivKey operator-(PrivKey const& o) const {
		auto tmp = *this;
		tmp -= o;
		return tmp;
	}
	PrivKey operator*(PrivKey const& o) const {
		auto tmp = *this;
		tmp *= o;
		return tmp;
	}

	static PrivKey from_buffer(std::uint8_t const buffer[32]) {
		return PrivKey(buffer);
	}
	void to_buffer(std::uint8_t buffer[32]) const {
		for (auto i = 0; i < 32; ++i)
			buffer[i] = key[i];
	}

	bool operator==(PrivKey const& o) const;
	bool operator!=(PrivKey const& o) const {
		return !(*this == o);
	}

	friend class PubKey;
	friend std::ostream& ::operator<<(std::ostream&, Secp256k1::PrivKey const&);

};

}

namespace S {
template<typename A>
void serialize(A& a, ::Secp256k1::PrivKey const& t) {
	std::uint8_t buffer[32];
	t.to_buffer(buffer);
	for (auto i = 0; i < 32; ++i)
		serialize(a, buffer[i]);
}
template<typename A>
void deserialize(A& a, ::Secp256k1::PrivKey& t) {
	std::uint8_t buffer[32];
	for (auto i = 0; i < 32; ++i)
		deserialize(a, buffer[i]);
	t = ::Secp256k1::PrivKey::from_buffer(buffer);
}
}

#endif /* CLDCB_COMMON_SECP256K1_PRIVKEY_HPP */

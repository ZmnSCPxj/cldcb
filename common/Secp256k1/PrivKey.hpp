#ifndef CLDCB_COMMON_SECP256K1_PRIVKEY_HPP
#define CLDCB_COMMON_SECP256K1_PRIVKEY_HPP

#include<cstdint>
#include<ostream>
#include<string>

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Secp256k1 { class Random; }
namespace Secp256k1 { class Signature; }

std::ostream& operator<<(std::ostream&, Secp256k1::PrivKey const&);

namespace Secp256k1 {

class PrivKey {
private:
	std::uint8_t key[32];

	PrivKey( std::uint8_t key_[32] );

public:
	PrivKey() =delete;

	/* Load private key from a hex-encoded string.  */
	explicit PrivKey(std::string const&);
	/* Pick a random private key.  */
	explicit PrivKey(Secp256k1::Random& rand);
	/* Copy an existing private key.  */
	PrivKey(PrivKey const&);

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

	static PrivKey from_buffer(std::uint8_t buffer[32]) {
		return PrivKey(buffer);
	}

	bool operator==(PrivKey const& o) const {
		for (auto i = 0; i < 32; ++i)
			if (key[i] != o.key[i])
				return false;
		return true;
	}
	bool operator!=(PrivKey const& o) const {
		return !(*this == o);
	}

	friend class PubKey;
	friend std::ostream& ::operator<<(std::ostream&, Secp256k1::PrivKey const&);

	/* TODO: serialization. */
};

}

#endif /* CLDCB_COMMON_SECP256K1_PRIVKEY_HPP */

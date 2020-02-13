#ifndef CLDCB_COMMON_SECP256K1_PRIVKEY_HPP
#define CLDCB_COMMON_SECP256K1_PRIVKEY_HPP

#include<cstdint>
#include<ostream>

#include"Secp256k1/Context.hpp"

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Secp256k1 { class Random; }
namespace Secp256k1 { class Signature; }

std::ostream& operator<<(std::ostream&, Secp256k1::PrivKey const&);

namespace Secp256k1 {

class PrivKey {
private:
	Secp256k1::Context ctx;
	std::uint8_t key[32];

	PrivKey( Secp256k1::Context const& ctx_
	       , std::uint8_t key_[32]
	       );

public:
	PrivKey() =delete;

	PrivKey( Secp256k1::Context const& ctx_
	       , Secp256k1::Random& rand
	       );
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

	static PrivKey from_buffer( Secp256k1::Context const& ctx
				  , std::uint8_t buffer[32]
				  ) {
		return PrivKey(ctx, buffer);
	}

	friend class PubKey;
	friend std::ostream& ::operator<<(std::ostream&, Secp256k1::PrivKey const&);

	/* TODO: serialization. */
};

}

#endif /* CLDCB_COMMON_SECP256K1_PRIVKEY_HPP */

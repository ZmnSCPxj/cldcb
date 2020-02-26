#ifndef CLDCB_COMMON_SECP256K1_PUBKEY_HPP
#define CLDCB_COMMON_SECP256K1_PUBKEY_HPP

#include<cstdint>
#include<istream>
#include<memory>
#include<ostream>
#include<stdexcept>
#include<string>
#include<utility>
#include"S.hpp"

extern "C" {
struct secp256k1_context_struct;
}

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }

std::ostream& operator<<(std::ostream&, Secp256k1::PubKey const&);

namespace Secp256k1 {

/* Thrown in case of being fed an invalid public key.  */
class InvalidPubKey : public std::invalid_argument {
public:
	InvalidPubKey() : std::invalid_argument("Invalid public key.") { }
};

class PubKey {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	explicit PubKey(secp256k1_context_struct *, std::uint8_t buffer[33]);
	explicit PubKey(std::uint8_t const buffer[33]);

public:
	/* Get G.  */
	PubKey();
	/* Load public key from a hex-encoded string.  */
	explicit PubKey(std::string const&);
	/* Get the public key behind the given private key.  */
	explicit PubKey(Secp256k1::PrivKey const&);

	/* Copy an existing public key.  */
	PubKey(PubKey const&);
	PubKey(PubKey&&);

	~PubKey();

	PubKey& operator=(PubKey const& o) {
		auto tmp = PubKey(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}
	PubKey& operator=(PubKey&& o) {
		auto tmp = PubKey(std::move(o));
		tmp.pimpl.swap(pimpl);
		return *this;
	}

	void negate();
	PubKey operator-() const {
		auto tmp = *this;
		tmp.negate();
		return tmp;
	}

	PubKey& operator+=(PubKey const&);
	PubKey operator+(PubKey const& o) const {
		auto tmp = *this;
		tmp += o;
		return tmp;
	}

	PubKey& operator-=(PubKey const& o) {
		return (*this += -o);
	}
	PubKey operator-(PubKey const& o) const {
		auto tmp = o;
		tmp.negate();
		tmp += *this;
		return tmp;
	}

	PubKey& operator*=(PrivKey const&);
	PubKey operator*(PrivKey const& o) const {
		auto tmp = *this;
		tmp *= o;
		return tmp;
	}

	bool operator==(PubKey const&) const;
	bool operator!=(PubKey const& o) const {
		return !(*this == o);
	}

	friend std::ostream& ::operator<<(std::ostream&, PubKey const&);

	static PubKey from_buffer(std::uint8_t const buffer[33]) {
		return PubKey(buffer);
	}
	/* Needed for the generator point.  */
	static PubKey from_buffer_with_context( secp256k1_context_struct *ctx
					      , std::uint8_t buffer[33]
					      ) {
		return PubKey(ctx, buffer);
	}

	void to_buffer(std::uint8_t buffer[33]) const;

	/* TODO: Serialization.  */
};

inline
PubKey operator*(PrivKey const& a, PubKey const& B) {
	return B * a;
}

}

namespace S {

template<typename A>
inline
void serialize(A& a, ::Secp256k1::PubKey const& pk) {
	std::uint8_t buffer[33];
	pk.to_buffer(buffer);
	for (auto i = 0; i < 33; ++i)
		put_byte(a, buffer[i]);
}
template<typename A>
inline
void deserialize(A& a, ::Secp256k1::PubKey& pk) {
	std::uint8_t buffer[33];
	buffer[0] = get_byte(a);
	if (buffer[0] != 0x02 && buffer[0] != 0x03)
		throw S::InvalidByte();
	for (auto i = 1; i < 33; ++i)
		buffer[i] = get_byte(a);
	pk = ::Secp256k1::PubKey::from_buffer(buffer);
}

}

#endif /* CLDCB_COMMON_SECP256K1_PUBKEY_HPP */

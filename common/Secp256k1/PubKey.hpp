#ifndef CLDCB_COMMON_SECP256K1_PUBKEY_HPP
#define CLDCB_COMMON_SECP256K1_PUBKEY_HPP

#include<cstdint>
#include<istream>
#include<memory>
#include<ostream>
#include<stdexcept>
#include<string>

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
	explicit PubKey(std::uint8_t buffer[33]);

public:
	/* Load public key from a hex-encoded string.  */
	explicit PubKey(std::string const&);
	/* Get the public key behind the given private key.  */
	explicit PubKey(Secp256k1::PrivKey const&);

	/* Copy an existing public key.  */
	PubKey(PubKey const&);
	PubKey(PubKey&&);

	~PubKey();

	PubKey& operator=(PubKey const&) =default;
	PubKey& operator=(PubKey&&) =default;

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

	static PubKey from_buffer(std::uint8_t buffer[33]) {
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

#endif /* CLDCB_COMMON_SECP256K1_PUBKEY_HPP */

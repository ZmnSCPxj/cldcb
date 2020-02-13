#ifndef CLDCB_SECP256K1_PUBKEY_HPP
#define CLDCB_SECP256K1_PUBKEY_HPP

#include<istream>
#include<memory>
#include<ostream>
#include<stdexcept>
#include"Secp256k1/Context.hpp"

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }

std::ostream& operator<<(std::ostream&, Secp256k1::PubKey const&);

namespace Secp256k1 {

/* Thrown if the PrivKey is somehow invalid.*/
class InvalidPrivKey : public std::runtime_error {
public:
	InvalidPrivKey() : std::runtime_error("Secp256k1: Invalid Privkey") { }
};

class PubKey {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	explicit PubKey(Secp256k1::PrivKey const&);

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

	/* TODO: Serialization.  */
};

inline
PubKey operator*(PrivKey const& a, PubKey const& B) {
	return B * a;
}

}

#endif /* CLDCB_SECP256K1_PUBKEY_HPP */

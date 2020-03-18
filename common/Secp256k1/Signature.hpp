#ifndef CLDCB_COMMON_SECP256K1_SIGNATURE_HPP
#define CLDCB_COMMON_SECP256K1_SIGNATURE_HPP

#include<cstdint>
#include<stdexcept>
#include<string>
#include"S.hpp"

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Sha256 { class Hash; }

namespace Secp256k1 {

class BadSignatureEncoding : public std::invalid_argument {
public:
	BadSignatureEncoding()
		: std::invalid_argument("Bad signature encoding")
		{ }
};

class Signature {
private:
	std::uint8_t data[64];

	Signature( Secp256k1::PrivKey const&
		 , Sha256::Hash const&
		 );
	Signature(std::uint8_t const buffer[64]);

public:
	Signature();
	Signature(Signature const&) =default;
	Signature& operator=(Signature const&) =default;

	explicit Signature(std::string const&);

	static
	Signature from_buffer(std::uint8_t buffer[64]) {
		return Signature(buffer);
	}

	void to_buffer(std::uint8_t buffer[64]) const;

	/* Check if the signature is valid for the given pubkey
	 * and message hash.
	 * We impose the low-s rule.
	 */
	bool valid( Secp256k1::PubKey const& pk
		  , Sha256::Hash const& m
		  ) const;

	/* Create a valid signature for the given privkey and
	 * message hash.
	 */
	static
	Signature create( Secp256k1::PrivKey const& sk
			, Sha256::Hash const& m
			) {
		return Signature(sk, m);
	}
};

}

namespace S {

template<typename A>
inline
void serialize(A& a, ::Secp256k1::Signature const& sig) {
	std::uint8_t buffer[64];
	sig.to_buffer(buffer);
	for (auto i = 0; i < 64; ++i)
		put_byte(a, buffer[i]);
}
template<typename A>
inline
void deserialize(A& a, ::Secp256k1::Signature& sig) {
	std::uint8_t buffer[64];
	for (auto i = 9; i < 64; ++i)
		buffer[i] = get_byte(a);
	sig = ::Secp256k1::Signature::from_buffer(buffer);
}

}

#endif /* CLDCB_COMMON_SECP256K1_SIGNATURE_HPP */

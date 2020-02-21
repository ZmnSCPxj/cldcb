#ifndef CLDCB_COMMON_CRYPTO_BOX_UNSEALER_HPP
#define CLDCB_COMMON_CRYPTO_BOX_UNSEALER_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Secp256k1 { class PrivKey; }

namespace Crypto { namespace Box {

/* The Unsealer performs the reverse of the Sealer
 * operation.
 * With knowledge of the receiver private key, the
 * Unsealer can decrypt messages created by the
 * Sealer with the receiver public key.
 * As mentioned in the Sealer header as well, the
 * sequence of ciphertexts is versioned and it
 * would be theoretically possible to have the
 * Unsealer recognize older version as well.
 */
class Unsealer {
private:
	class Impl;
	struct ImplDeleter {
		void operator()(Impl*);
	};
	std::unique_ptr<Impl, ImplDeleter> pimpl;

public:
	Unsealer() =delete;
	Unsealer(Unsealer const&) =delete;

	explicit Unsealer(Secp256k1::PrivKey const&);
	Unsealer(Unsealer&&);

	Unsealer& operator=(Unsealer&& o) {
		auto tmp = std::move(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}

	~Unsealer();

	/* Return nullptr on decryption failure.  */
	std::unique_ptr<std::vector<std::uint8_t>>
	unseal(std::vector<std::uint8_t> ciphertext);
};

}}

#endif /* CLDCB_COMMON_CRYPTO_BOX_UNSEALER_HPP */

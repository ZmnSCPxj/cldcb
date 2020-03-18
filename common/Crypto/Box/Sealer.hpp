#ifndef CLDCB_COMMON_CRYPTO_BOX_SEALER_HPP
#define CLDCB_COMMON_CRYPTO_BOX_SEALER_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }

namespace Crypto { namespace Box {

/* A Sealer accepts plaintext, then encrypts it to a
 * public key, proving a particular sender generated
 * the ciphertext.
 * The resulting ciphertext can only be opened with
 * knowledge of the corresponding private key, and
 * that it came from a particular sender.
 *
 * The intent is that you construct a Sealer for a
 * set of related messages, then use the same instance
 * over and over again.
 * The corresponding Unsealer must also be used over
 * and over again similarly.
 *
 * The ciphertext output is versioned, thus it may be
 * possible to upgrade the Sealer / Unsealer such
 * that newer Sealer versions can change the encryption
 * algorithm (in case a cryptographic break in the
 * current algorithm) and newer Unsealers can still
 * decrypt previous ciphertexts.
 */
class Sealer {
private:
	class Impl;
	struct ImplDeleter {
		void operator()(Impl*);
	};
	std::unique_ptr<Impl, ImplDeleter> pimpl;

public:
	Sealer() =delete;
	Sealer(Sealer const&) =delete;

	explicit Sealer( Secp256k1::PrivKey const& sender
		       , Secp256k1::PubKey const& receiver
		       );
	Sealer(Sealer&&);

	Sealer& operator=(Sealer&& o) {
		auto tmp = std::move(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}

	~Sealer();

	std::vector<std::uint8_t>
	seal(std::vector<std::uint8_t> plaintext);
};

}}

#endif /* CLDCB_COMMON_CRYPTO_BOX_SEALER_HPP */

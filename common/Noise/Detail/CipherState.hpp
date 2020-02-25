#ifndef CLDCB_COMMON_NOISE_DETAIL_CIPHERSTATE_HPP
#define CLDCB_COMMON_NOISE_DETAIL_CIPHERSTATE_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Crypto { class Secret; }

namespace Noise { namespace Detail {

/* A Noise CipherState object for
 * Noise_XK_secp256k1_ChaChaPoly_SHA256
 */
class CipherState {
private:
	std::unique_ptr<Crypto::Secret> k;
	std::uint64_t n;
	bool wrapped;

public:
	CipherState() : k(nullptr), n(0), wrapped(false) {}
	CipherState(CipherState const&);
	CipherState(CipherState&&) =default;
	CipherState& operator=(CipherState const&) =default;
	CipherState& operator=(CipherState&&) =default;
	~CipherState();

	void initialize_key(Crypto::Secret const& k_);
	bool has_key() const {
		if (k)
			return true;
		else
			return false;
	}
	void set_nonce(uint64_t n_) { n = n_; }
	uint64_t get_nonce() const { return n; }

	bool can_encrypt() const { return !wrapped; }
	std::vector<std::uint8_t>
	encrypt_with_ad( std::vector<std::uint8_t> const& ad
		       , std::vector<std::uint8_t> const& plaintext
		       );
	std::vector<std::uint8_t>
	decrypt_with_ad( std::vector<std::uint8_t> const& ad
		       , std::vector<std::uint8_t> const& ciphertext
		       );
};

}}

#endif /* CLDCB_COMMON_NOISE_DETAIL_CIPHERSTATE_HPP */

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
	explicit
	CipherState( Crypto::Secret const& k_
		   ) : k(nullptr)
		     , n(0)
		     , wrapped(false)
		     {
		initialize_key(k_);
	}
	CipherState(CipherState const&);
	CipherState(CipherState&&) =default;
	CipherState& operator=(CipherState const& o) {
		auto tmp = CipherState(o);
		k.swap(tmp.k);
		n = tmp.n;
		wrapped = tmp.wrapped;
		return *this;
	}
	CipherState& operator=(CipherState&&) =default;
	~CipherState();

	void initialize_key(Crypto::Secret const& k_);
	std::unique_ptr<Crypto::Secret> const& get_key() const { return k; }
	bool has_key() const {
		if (k)
			return true;
		else
			return false;
	}
	void set_nonce(uint64_t n_) { n = n_; wrapped = false; }
	uint64_t get_nonce() const { return n; }

	bool can_encrypt() const { return !wrapped; }
	std::vector<std::uint8_t>
	encrypt_with_ad( std::vector<std::uint8_t> const& ad
		       , std::vector<std::uint8_t> const& plaintext
		       );
	/* Return nullptr on decryption failure.  */
	std::unique_ptr<std::vector<std::uint8_t>>
	decrypt_with_ad( std::vector<std::uint8_t> const& ad
		       , std::vector<std::uint8_t> const& ciphertext
		       );
};

}}

#endif /* CLDCB_COMMON_NOISE_DETAIL_CIPHERSTATE_HPP */

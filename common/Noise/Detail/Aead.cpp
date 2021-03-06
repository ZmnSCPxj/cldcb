#include<assert.h>
#include<sodium/crypto_aead_chacha20poly1305.h>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/Aead.hpp"
#include"Util/make_unique.hpp"

namespace {

void encode_nonce(std::uint8_t encoded_nonce[12], std::uint64_t n) {
	for (auto i = 0; i < 4; ++i)
		encoded_nonce[i] = 0;
	encoded_nonce[0 + 4] = std::uint8_t((n >>  0) & 0xff);
	encoded_nonce[1 + 4] = std::uint8_t((n >>  8) & 0xff);
	encoded_nonce[2 + 4] = std::uint8_t((n >> 16) & 0xff);
	encoded_nonce[3 + 4] = std::uint8_t((n >> 24) & 0xff);
	encoded_nonce[4 + 4] = std::uint8_t((n >> 32) & 0xff);
	encoded_nonce[5 + 4] = std::uint8_t((n >> 40) & 0xff);
	encoded_nonce[6 + 4] = std::uint8_t((n >> 48) & 0xff);
	encoded_nonce[7 + 4] = std::uint8_t((n >> 56) & 0xff);
}

}

namespace Noise { namespace Detail { namespace Aead {


std::vector<std::uint8_t>
encrypt( Crypto::Secret const& k
       , std::uint64_t n
       , std::vector<std::uint8_t> const& ad
       , std::vector<std::uint8_t> const& plaintext
       ) {
	std::uint8_t nonce[12];
	encode_nonce(nonce, n);

	auto ciphertext = std::vector<std::uint8_t>( plaintext.size()
						   + crypto_aead_chacha20poly1305_ietf_ABYTES
						   );

	auto res = crypto_aead_chacha20poly1305_ietf_encrypt
		( &ciphertext[0]
		, NULL /* length of ciphertext -- already known */
		, &plaintext[0]
		, plaintext.size()
		, &ad[0]
		, ad.size()
		, NULL /* secret nonce -- inapplicable to chacha20poly1305 */
		, nonce
		, k.get_buffer()
		);
	assert(res == 0);

	return ciphertext;
}
std::unique_ptr<std::vector<std::uint8_t>>
decrypt( Crypto::Secret const& k
       , std::uint64_t n
       , std::vector<std::uint8_t> const& ad
       , std::vector<std::uint8_t> const& ciphertext
       ) {
	assert(ciphertext.size() >= crypto_aead_chacha20poly1305_ietf_ABYTES);

	std::uint8_t nonce[12];
	encode_nonce(nonce, n);

	auto plaintext = std::vector<std::uint8_t>( ciphertext.size()
						  - crypto_aead_chacha20poly1305_ietf_ABYTES
						  );

	auto res = crypto_aead_chacha20poly1305_ietf_decrypt
		( &plaintext[0]
		, NULL /* length of plaintext -- already known */
		, NULL /* secret nonce -- inapplicable to chacha20poly1305 */
		, &ciphertext[0]
		, ciphertext.size()
		, &ad[0]
		, ad.size()
		, nonce
		, k.get_buffer()
		);
	if (res != 0)
		return nullptr;

	return Util::make_unique<std::vector<std::uint8_t>>(std::move(plaintext));
}

}}}

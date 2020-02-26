#include<assert.h>
#include<stdexcept>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/Aead.hpp"
#include"Noise/Detail/CipherState.hpp"
#include"Util/make_unique.hpp"

namespace Noise { namespace Detail {

CipherState::CipherState(CipherState const& o) {
	if (o.k)
		k = Util::make_unique<Crypto::Secret>(*o.k);
	else
		k = nullptr;
	n = o.n;
	wrapped = o.wrapped;
}
CipherState::~CipherState() { }

void CipherState::initialize_key(Crypto::Secret const& k_) {
	k = Util::make_unique<Crypto::Secret>(k_);
	n = 0;
	wrapped = false;
}
std::vector<std::uint8_t>
CipherState::encrypt_with_ad( std::vector<std::uint8_t> const& ad
			    , std::vector<std::uint8_t> const& plaintext
			    ) {
	if (!k)
		return plaintext;

	if (wrapped)
		throw std::range_error("Noise::Detail::CipherState::nonce already wrapped around, unsafe to encrypt");

	auto ciphertext = Noise::Detail::Aead::encrypt(*k, n, ad, plaintext);

	++n;
	if (n == 0)
		wrapped = true;

	return ciphertext;
}
std::unique_ptr<std::vector<std::uint8_t>>
CipherState::decrypt_with_ad( std::vector<std::uint8_t> const& ad
			    , std::vector<std::uint8_t> const& ciphertext
			    ) {
	if (!k)
		return Util::make_unique<std::vector<std::uint8_t>>(ciphertext);

	if (wrapped)
		throw std::range_error("Noise::Detail::CipherState::nonce already wrapped around, unsafe to encrypt");

	auto plaintext = Noise::Detail::Aead::decrypt(*k, n, ad, ciphertext);
	if (!plaintext)
		return plaintext;

	++n;
	if (n == 0)
		wrapped = true;

	return plaintext;
}

}}


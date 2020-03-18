#include<assert.h>
#include<sodium/crypto_aead_chacha20poly1305.h>
#include<sodium/utils.h>
#include<stdexcept>
#include"Crypto/Box/Detail/Nonce.hpp"
#include"Crypto/Box/Unsealer.hpp"
#include"Crypto/Secret.hpp"
#include"S.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Signature.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include"Util/make_unique.hpp"

namespace Crypto { namespace Box {

class Unsealer::Impl {
private:
	Secp256k1::PubKey sender_pubkey;
	Secp256k1::PrivKey receiver_privkey;

	std::uint8_t shared_secret_storage[sizeof(Crypto::Secret)];

	Detail::Nonce nonce;

	bool start;

	Impl( Secp256k1::PubKey const& sender_pubkey_
	    , Secp256k1::PrivKey const& receiver_privkey_
	    )
		: sender_pubkey(sender_pubkey_)
		, receiver_privkey(receiver_privkey_)
		, nonce()
		, start(true)
		{
	}

	Crypto::Secret const& shared_secret() const {
		assert(!start);
		return *reinterpret_cast<Crypto::Secret const*>(
			shared_secret_storage
		);
	}

public:
	Impl() =delete;

	~Impl() {
		if (!start)
			shared_secret().~Secret();
	}

	static
	std::unique_ptr<Impl, ImplDeleter>
	create( Secp256k1::PubKey const& sender_pubkey
	      , Secp256k1::PrivKey const& receiver_privkey
	      ) {
		/* FIXME: Use some kind of secure memory. */
		return std::unique_ptr<Impl, ImplDeleter>(
			new Impl(sender_pubkey, receiver_privkey)
		);
	}

	std::unique_ptr<std::vector<std::uint8_t>>
	unseal(std::vector<std::uint8_t> ciphertext) {
		auto ret = std::vector<std::uint8_t>();
		auto msg = (std::uint8_t const*) nullptr;
		auto msglen = std::size_t(0);
		if (start) {
			if (ciphertext.size() < crypto_aead_chacha20poly1305_ietf_ABYTES + 33 + 64)
				return nullptr;
			/* Check version.  */
			if (ciphertext[0] != 0x02 && ciphertext[0] != 0x03)
				return nullptr;

			/* First 33 bytes are an ephemeral pubkey.  */
			auto ephemeral_pubkey = Secp256k1::PubKey::from_buffer(&ciphertext[0]);
			/* Next 64 bytes are a signature.  */
			auto s = Secp256k1::Signature::from_buffer(&ciphertext[33]);

			/* Validate the ephmeral pubkey.  */
			auto m = Sha256::fun(&ciphertext[0], 33);
			if (!s.valid(sender_pubkey, m))
				return nullptr;

			/* Derive shared secret.  */
			new(shared_secret_storage) Crypto::Secret(
				Secp256k1::ecdh(receiver_privkey, ephemeral_pubkey)
			);
			start = false;

			msg = &ciphertext[33 + 64];
			msglen = ciphertext.size() - 33 - 64;
		} else {
			if (ciphertext.size() < crypto_aead_chacha20poly1305_ietf_ABYTES)
				return nullptr;
			msg = &ciphertext[0];
			msglen = ciphertext.size();
		}

		ret.resize(msglen - crypto_aead_chacha20poly1305_ietf_ABYTES);

		auto res = crypto_aead_chacha20poly1305_ietf_decrypt
			( &ret[0] /* plaintext output */
			, NULL /* plaintext length output -- known */
			, NULL /* secret nonce -- nonexistent for chacha20 */
			, msg /* ciphertext + tag */
			, msglen
			, (unsigned char const*) "CLDCB 00" /* associated data */
			, 8 /* associated data length */
			, nonce.get() /* public nonce */
			, shared_secret().get_buffer() /* encryption key */
			);
		if (res != 0)
			return nullptr;

		++nonce;

		return Util::make_unique<std::vector<std::uint8_t>>(std::move(ret));
	}
};

void Unsealer::ImplDeleter::operator()(Impl* p) {
	/* FIXME: Use some kind of secure memory. */
	delete p;
}

Unsealer::Unsealer( Secp256k1::PubKey const& sender_pubkey
		  , Secp256k1::PrivKey const& receiver_privkey
		  )
	: pimpl(Impl::create(sender_pubkey, receiver_privkey)) { }

Unsealer::Unsealer(Unsealer&& o) {
	o.pimpl.swap(pimpl);
}
Unsealer::~Unsealer() { }

std::unique_ptr<std::vector<std::uint8_t>>
Unsealer::unseal(std::vector<std::uint8_t> ciphertext) {
	if (!pimpl)
		throw std::logic_error("Use of Crypto::Box::Unsealer after being moved from.");
	return pimpl->unseal(std::move(ciphertext));
}

}}

#include<assert.h>
#include<sodium/crypto_aead_chacha20poly1305.h>
#include<sodium/utils.h>
#include<stdexcept>
#include"Crypto/Box/Detail/Nonce.hpp"
#include"Crypto/Box/Sealer.hpp"
#include"Crypto/Secret.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Secp256k1/Signature.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Crypto { namespace Box {

class Sealer::Impl {
private:
	/* These must be declared in the specific order below,
	 * since they must be constructed in order.
	 */
	Secp256k1::Random rand;
	Secp256k1::PrivKey ephemeral_privkey;
	Crypto::Secret shared_secret;

	Secp256k1::PrivKey sender_privkey;

	/* nonce counters.  */
	Detail::Nonce nonce;

	bool start;

	Impl( Secp256k1::PrivKey const& sender_privkey_
	    , Secp256k1::PubKey const& receiver_pubkey
	    )
		: rand()
		, ephemeral_privkey(rand)
		, shared_secret( Secp256k1::ecdh( ephemeral_privkey
						, receiver_pubkey
						)
			       )
		, sender_privkey(sender_privkey_)
		, nonce()
		, start(true)
		{
	}


public:
	Impl() =delete;

	static
	std::unique_ptr<Impl, ImplDeleter>
	create( Secp256k1::PrivKey const& sender_privkey
	      , Secp256k1::PubKey const& receiver_pubkey
	      ) {
		/* FIXME: Use some kind of secure memory. */
		return std::unique_ptr<Impl, ImplDeleter>(
			new Impl(sender_privkey, receiver_pubkey)
		);
	}

	std::vector<std::uint8_t>
	seal(std::vector<std::uint8_t> plaintext) {
		auto ret = std::vector<std::uint8_t>();
		auto msg = (std::uint8_t *) nullptr;
		if (start) {
			ret.resize( 33 /* ephemeral pubkey */
				  + 64 /* signature of the ephemeral pubkey */
				  + plaintext.size()
				  + crypto_aead_chacha20poly1305_ietf_ABYTES
				  );
			msg = &ret[33 + 64];
			auto ephemeral_pubkey = Secp256k1::PubKey(ephemeral_privkey);
			ephemeral_pubkey.to_buffer(&ret[0]);

			/* The first byte of the above will be either
			 * 0x02 or 0x03.
			 * This doubles as a version tag: if the first
			 * byte of the first ciphertext is not either
			 * 0x02 or 0x03, then it is a new version of
			 * the Crypto::Box protocol.
			 */

			/* Generate the signature.  */
			auto m = Sha256::fun(&ret[0], 33);
			auto s = Secp256k1::Signature::create( sender_privkey
							     , m
							     );
			s.to_buffer(&ret[33]);

			start = false;
		} else {
			ret.resize( plaintext.size()
				  + crypto_aead_chacha20poly1305_ietf_ABYTES
				  );
			msg = &ret[0];
		}

		auto res = crypto_aead_chacha20poly1305_ietf_encrypt
			( msg /* ciphertext + tag.  */
			, NULL /* ciphertext length output -- known */
			, &plaintext[0]
			, plaintext.size()
			, (unsigned char const *) "CLDCB 00" /* associated data */
			, 8 /* Length of associated data */
			, NULL /* secret nonce -- nonexistent for chacha20 */
			, nonce.get() /* public nonce */
			, shared_secret.get_buffer() /* encryption key */
			);
		assert(res == 0);

		++nonce;

		return ret;
	}

};

void Sealer::ImplDeleter::operator()(Impl* p) {
	/* FIXME: Use some kind of secure memory. */
	delete p;
}

Sealer::Sealer( Secp256k1::PrivKey const& sender_privkey
	      , Secp256k1::PubKey const& receiver_pubkey
	      )
	: pimpl(Impl::create(sender_privkey, receiver_pubkey)) { }
Sealer::Sealer(Sealer&& o) {
	pimpl.swap(o.pimpl);
}

Sealer::~Sealer() { }

std::vector<std::uint8_t>
Sealer::seal(std::vector<std::uint8_t> plaintext) {
	if (!pimpl)
		/* FIXME: backtrace-extracting exception.  */
		throw std::logic_error("Use of a Crypto::Box::Sealer after ithas been moved from.");
	return pimpl->seal(std::move(plaintext));
}

}}

#include<assert.h>
#include<secp256k1.h>
#include<string.h>
#include"Secp256k1/Detail/context.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Signature.hpp"
#include"Sha256/Hash.hpp"
#include"Util/Str.hpp"

using Secp256k1::Detail::context;

namespace Secp256k1 {

Signature::Signature(std::uint8_t const buffer[64]) {
	auto res = secp256k1_ecdsa_signature_parse_compact
		( context.get()
		, reinterpret_cast<secp256k1_ecdsa_signature*>(data)
		, reinterpret_cast<const unsigned char*>(buffer)
		);
	if (res == 0)
		throw BadSignatureEncoding();
}
Signature::Signature( Secp256k1::PrivKey const& sk
		    , Sha256::Hash const& m
		    ) {
	std::uint8_t mbuf[32];
	m.to_buffer(mbuf);

	auto res = secp256k1_ecdsa_sign
		( context.get()
		, reinterpret_cast<secp256k1_ecdsa_signature*>(data)
		, reinterpret_cast<const unsigned char*>(mbuf)
		, reinterpret_cast<const unsigned char*>(sk.key)
		, nullptr
		, nullptr
		);
	if (res == 0)
		/* Extremely unlikely to happen.
		 * TODO: backtrace-capturing.
		 */
		throw std::runtime_error("Nonce generation for signing failed.");
}

Signature::Signature() {
	unsigned char buffer[64];
	memset(buffer, 0, 64);
	auto res = secp256k1_ecdsa_signature_parse_compact
		( context.get()
		, reinterpret_cast<secp256k1_ecdsa_signature*>(data)
		, buffer
		);
	if (res)
		return;
	else
		return;
}

Signature::Signature(std::string const& s) {
	auto buf = Util::Str::hexread(s);
	if (buf.size() != 64)
		throw BadSignatureEncoding();
	auto res = secp256k1_ecdsa_signature_parse_compact
		( context.get()
		, reinterpret_cast<secp256k1_ecdsa_signature*>(data)
		, reinterpret_cast<const unsigned char*>(&buf[0])
		);
	if (res == 0)
		throw BadSignatureEncoding();
}

void Signature::to_buffer(std::uint8_t buffer[64]) const {
	auto res = secp256k1_ecdsa_signature_serialize_compact
		( context.get()
		, reinterpret_cast<unsigned char*>(buffer)
		, reinterpret_cast<const secp256k1_ecdsa_signature*>(data)
		);
	assert(res == 1);
}

bool Signature::valid( Secp256k1::PubKey const& pk
		     , Sha256::Hash const& m
		     ) const {
	std::uint8_t mbuf[32];
	m.to_buffer(mbuf);

	auto res = secp256k1_ecdsa_verify
		( context.get()
		, reinterpret_cast<const secp256k1_ecdsa_signature*>(data)
		, reinterpret_cast<const unsigned char*>(mbuf)
		, reinterpret_cast<const secp256k1_pubkey*>(pk.get_key())
		);
	return res != 0;
}

}

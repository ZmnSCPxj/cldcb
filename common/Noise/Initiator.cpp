#include<algorithm>
#include<assert.h>
#include<stdexcept>
#include"Noise/Detail/Aead.hpp"
#include"Noise/Detail/HS.hpp"
#include"Noise/Encryptor.hpp"
#include"Noise/Initiator.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Util/make_unique.hpp"

namespace {

auto const nulldata = std::vector<std::uint8_t>(0);

/* Puts a 32-byte secret into an std::vector.  */
std::vector<std::uint8_t> vectorize_secret(Crypto::Secret const& h) {
	auto ret = std::vector<std::uint8_t>(32);
	h.to_buffer(&ret[0]);
	return ret;
}
/* Puts a 33-byte pubkey into an std::vector.  */
std::vector<std::uint8_t> vectorize_pubkey(Secp256k1::PubKey const& pk) {
	auto ret = std::vector<std::uint8_t>(33);
	pk.to_buffer(&ret[0]);
	return ret;
}

}

namespace Noise {

class Initiator::Impl {
private:
	Detail::HS handshake;

	/* Local static key.  */
	Secp256k1::PrivKey const& s;
	Secp256k1::PubKey s_pub;

	/* Remote static key*/
	Secp256k1::PubKey rs;

	/* Local ephemeral keypair.  */
	Secp256k1::PrivKey e;
	Secp256k1::PubKey e_pub;

	int act;

public:
	Impl( Secp256k1::PrivKey const& s_
	    , Secp256k1::PubKey const& rs_
	    , Secp256k1::PrivKey const& e_
	    , std::string const& prologue
	    , std::string const& protocol_name
	    ) : handshake(prologue, protocol_name)
	      , s(s_)
	      , s_pub(s_)
	      , rs(rs_)
	      , e(e_)
	      , e_pub(e_)
	      , act(1)
	      { }

	std::vector<std::uint8_t> act1() {
		if (act != 1)
			throw std::logic_error("Noise::Initiator: act 1 called again.");

		/* As a concluding step, both sides mix the responder's
		 * public key into the handshake digest:
		 */
		{
			std::uint8_t buffer[33];
			rs.to_buffer(buffer);
			handshake.mix_h(buffer, sizeof(buffer));
		}

		act = 2;

		/* 2. `h = SHA-256(h || e.pub.serializeCompressed())` */
		{
			std::uint8_t buffer[33];
			e_pub.to_buffer(buffer);
			handshake.mix_h(buffer, sizeof(buffer));
		}
		/* 3. `es = ECDH(e.priv, rs)` */
		auto es = Secp256k1::ecdh(e, rs);
		/* 4. `ck, temp_k1 = HKDF(ck, es)` */
		auto temp_k1 = handshake.mix_ck(es);
		/* 5. `c = encryptWithAD(temp_k1, 0, h, zero)` */
		auto c = Noise::Detail::Aead::encrypt( temp_k1
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , nulldata
						     );
		/* 6. `h = SHA-256(h || c)` */
		handshake.mix_h(&c[0], c.size());
		/* 7. Send `m = 0 || e.pub.serializeCompressed() || c` to the responder over the network buffer. */
		auto ret = std::vector<std::uint8_t>( 1 /* 0 version byte.  */
						    + 33 /* e.pub.  */
						    + c.size()
						    );
		ret[0] = 0;
		e_pub.to_buffer(&ret[1]);
		std::copy(c.begin(), c.end(), ret.begin() + 34);

		assert(ret.size() == 50);

		return ret;
	}

	std::unique_ptr<std::vector<std::uint8_t>>
	act2_and_3(std::vector<std::uint8_t> const& act2) {
		if (act != 2)
			throw std::logic_error("Noise::Initiator: act 2 and 3 called at incorrect time.");

		/* Act 2 Receiver.  */
		/* 1. Read _exactly_ 50 bytes from the network buffer. */
		if (act2.size() != 50)
			throw std::logic_error("Noise::Initiator: act 2 of incorrect size.");
		/* 2. Parse the read message (`m`) into `v`, `re`, and `c`: */
		auto v = act2[0];
		auto re_data = std::vector<std::uint8_t>(act2.begin() + 1, act2.begin() + 1 + 33);
		auto c_act2 = std::vector<std::uint8_t>(act2.begin() + 1 + 33, act2.end());
		/* 3. If `v` is an unrecognized handshake version, then the responder MUST
   		 * abort the connection attempt.
		 */
		if (v != 0)
			return nullptr;
		if (re_data[0] != 0x02 && re_data[0] != 0x03)
			return nullptr;
		auto re = Secp256k1::PubKey::from_buffer(&re_data[0]);
		/* 4. `h = SHA-256(h || re.serializeCompressed())` */
		handshake.mix_h(&act2[1], 33);
		/* 5. `ee = ECDH(e.priv, re)` */
		auto ee = Secp256k1::ecdh(e, re);
		/* 6. `ck, temp_k2 = HKDF(ck, ee)` */
		auto temp_k2 = handshake.mix_ck(ee);
		/* 7. `p = decryptWithAD(temp_k2, 0, h, c)` */
		auto p = Noise::Detail::Aead::decrypt( temp_k2
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , c_act2
						     );
		if (!p)
			return nullptr;
		assert(*p == nulldata);
		/* 8. `h = SHA-256(h || c)` */
		handshake.mix_h(&c_act2[0], c_act2.size());

		act = 3;

		/* Act 3 Sender.  */
		/* 1. `c = encryptWithAD(temp_k2, 1, h, s.pub.serializeCompressed())` */
		auto c = Noise::Detail::Aead::encrypt( temp_k2
						     , 1
						     , vectorize_secret(handshake.get_h())
						     , vectorize_pubkey(s_pub)
						     );
		/* 2. `h = SHA-256(h || c)` */
		handshake.mix_h(&c[0], c.size());
		/* 3. `se = ECDH(s.priv, re)` */
		auto se = Secp256k1::ecdh(s, re);
		/* 4. `ck, temp_k3 = HKDF(ck, se)` */
		auto temp_k3 = handshake.mix_ck(se);
		/* 5. `t = encryptWithAD(temp_k3, 0, h, zero)` */
		auto t = Noise::Detail::Aead::encrypt( temp_k3
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , nulldata
						     );
		/* Steps 6 and 7 are effectively done by get_encryptor.  */

		/* 8. Send `m = 0 || c || t` over the network buffer. */
		auto ret = std::vector<std::uint8_t>(1 + c.size() + t.size());
		ret[0] = 0;
		std::copy(c.begin(), c.end(), ret.begin() + 1);
		std::copy(t.begin(), t.end(), ret.begin() + 1 + c.size());

		assert(ret.size() == 66);

		return Util::make_unique<std::vector<std::uint8_t>>(std::move(ret));
	}

	Noise::Encryptor get_encryptor() {
		if (act != 3)
			throw std::logic_error("Noise::Initiator: get encryptor can only be called after act 3.");

		act = -1;

		/* 6. `sk, rk = HKDF(ck, zero)` */
		/* 7. `rn = 0, sn = 0` */
		auto split = handshake.split_ck();
		auto& sk = split.first;
		auto& rk = split.second;

		auto r = Noise::Detail::CipherState();
		auto s = Noise::Detail::CipherState();
		r.initialize_key(rk); /* sets rn = 0.  */
		s.initialize_key(sk); /* sets sn = 0.  */

		return Noise::Encryptor( std::move(r)
				       , std::move(s)
				       , handshake.get_ck()
				       );
	}

};

Initiator::Initiator( Secp256k1::PrivKey const& s
		    , Secp256k1::PubKey const& rs
		    , Secp256k1::PrivKey const& e
		    , std::string const& prologue
		    , std::string const& protocol_name
		    ) : pimpl(Util::make_unique<Impl>( s, rs, e
						     , prologue, protocol_name
						     ))
		      { }
Initiator::Initiator(Initiator&& o) : pimpl(std::move(o.pimpl)) { }
Initiator::~Initiator() { }

std::vector<std::uint8_t> Initiator::act1() {
	if (!pimpl)
		throw std::logic_error("Noise::Initiator: misuse of act1().");
	return pimpl->act1();
}

std::unique_ptr<std::vector<std::uint8_t>>
Initiator::act2_and_3(std::vector<std::uint8_t> const& act2) {
	if (!pimpl)
		throw std::logic_error("Noise::Initiator: misuse of act2_and_3().");
	auto ret = pimpl->act2_and_3(act2);
	/* Put us in a bad state.  */
	if (!ret)
		pimpl = nullptr;
	return ret;
}
Noise::Encryptor Initiator::get_encryptor() {
	if (!pimpl)
		throw std::logic_error("Noise::Initiator: misuse of get_encryptor().");
	auto ret = pimpl->get_encryptor();
	pimpl = nullptr;
	return ret;
}

}

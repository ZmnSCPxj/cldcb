#include<algorithm>
#include<assert.h>
#include<stdexcept>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/Aead.hpp"
#include"Noise/Detail/HS.hpp"
#include"Noise/Encryptor.hpp"
#include"Noise/Responder.hpp"
#include"Secp256k1/KeyPair.hpp"
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

}

namespace Noise {

class Responder::Impl {
private:
	Noise::Detail::HS handshake;

	Secp256k1::KeyPair const& s;

	Secp256k1::KeyPair e;

	/* Needed between acts 2 and 3.  */
	Crypto::Secret temp_k2;

	int act;

public:
	Impl() =delete;
	Impl( Secp256k1::KeyPair const& s_
	    , Secp256k1::KeyPair const& e_
	    , std::string const& prologue
	    , std::string const& protocol_name
	    ) : handshake(prologue, protocol_name)
	      , s(s_)
	      , e(e_)
	      , temp_k2()
	      , act(1)
	      { }

	std::unique_ptr<std::vector<std::uint8_t>>
	act1_and_2(std::vector<std::uint8_t> const& act1) {
		if (act != 1)
			throw std::logic_error("Noise::Responder: act 1 called again.");
		/* As a concluding step, both sides mix the responder's
		 * public key into the handshake digest:
		 */
		{
			std::uint8_t buffer[33];
			s.pub().to_buffer(buffer);
			handshake.mix_h(buffer, sizeof(buffer));
		}

		/* Act 1 Receiver.  */
		/* 1. Read _exactly_ 50 bytes from the network buffer. */
		if (act1.size() != 50)
			throw std::logic_error("Noise::Responder: incorrect size for act 1.");
		/* 2. Parse the read message (`m`) into `v`, `re`, and `c`: */
		auto v = act1[0];
		auto re_data = std::vector<std::uint8_t>(act1.begin() + 1, act1.begin() + 1 + 33);
		auto c_act1 = std::vector<std::uint8_t>(act1.begin() + 1 + 33, act1.end());
		/* 3. If `v` is an unrecognized handshake version, then the responder MUST
		 *     abort the connection attempt.
		 */
		if (v != 0)
			return nullptr;
		if (re_data[0] != 0x02 && re_data[0] != 0x03)
			return nullptr;
		auto re = Secp256k1::PubKey();
		try {
			re = Secp256k1::PubKey::from_buffer(&re_data[0]);
		} catch (Secp256k1::InvalidPubKey const&) {
			return nullptr;
		}
		/* 4. `h = SHA-256(h || re.serializeCompressed())` */
		handshake.mix_h(&re_data[0], re_data.size());
		/* 5. `es = ECDH(s.priv, re)` */
		auto es = Secp256k1::ecdh(s.priv(), re);
		/* 6. `ck, temp_k1 = HKDF(ck, es)` */
		auto temp_k1 = handshake.mix_ck(es);
		/* 7. `p = decryptWithAD(temp_k1, 0, h, c)` */
		auto p = Noise::Detail::Aead::decrypt( temp_k1
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , c_act1
						     );
		if (!p)
			return nullptr;
		assert(*p == nulldata);
		/* 8. `h = SHA-256(h || c)` */
		handshake.mix_h(&c_act1[0], c_act1.size());

		act = 2;

		/* Act 2 Sender.  */
		/* 2. `h = SHA-256(h || e.pub.serializeCompressed())` */
		{
			std::uint8_t buffer[33];
			e.pub().to_buffer(buffer);
			handshake.mix_h(buffer, sizeof(buffer));
		}
		/* 3. `ee = ECDH(e.priv, re)` */
		auto ee = Secp256k1::ecdh(e.priv(), re);
		/* 4. `ck, temp_k2 = HKDF(ck, ee)` */
		temp_k2 = handshake.mix_ck(ee);
		/* 5. `c = encryptWithAD(temp_k2, 0, h, zero)` */
		auto c = Noise::Detail::Aead::encrypt( temp_k2
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , nulldata
						     );
		/* 6. `h = SHA-256(h || c)` */
		handshake.mix_h(&c[0], c.size());
		/* 7. Send `m = 0 || e.pub.serializeCompressed() || c` to the initiator over the network buffer. */
		auto ret = std::vector<std::uint8_t>( 1 /* 0x00 version */
						    + 33 /* e pubkey.  */
						    + c.size()
						    );
		ret[0] = 0;
		e.pub().to_buffer(&ret[1]);
		std::copy(c.begin(), c.end(), ret.begin() + 1 + 33);

		assert(ret.size() == 50);

		act = 3;

		return Util::make_unique<std::vector<std::uint8_t>>(std::move(ret));
	}

	std::unique_ptr<Secp256k1::PubKey>
	act3(std::vector<std::uint8_t> const& act3) {
		if (act != 3)
			throw std::logic_error("Noise::Responder: act 3 incorrectly called.");

		/* Act 3 Receiver.  */
		/* 1. Read _exactly_ 66 bytes from the network buffer. */
		if (act3.size() != 66)
			throw std::logic_error("Noise::Responder: incorrect size for act 3.");
		/* 2. Parse the read message (`m`) into `v`, `c`, and `t`: */
		auto v = act3[0];
		auto c = std::vector<std::uint8_t>(act3.begin() + 1, act3.begin() + 1 + 49);
		auto t = std::vector<std::uint8_t>(act3.begin() + 1 + 49, act3.end());
		/* 3. If `v` is an unrecognized handshake version, then the responder MUST
		 *   abort the connection attempt.
		 */
		if (v != 0)
			return nullptr;
		/* 4. `rs = decryptWithAD(temp_k2, 1, h, c)` */
		auto p_rs_data = Noise::Detail::Aead::decrypt( temp_k2
							     , 1
							     , vectorize_secret(handshake.get_h())
							     , c
							     );
		if (!p_rs_data)
			return nullptr;
		auto& rs_data = *p_rs_data;
		if (rs_data[0] != 0x02 && rs_data[0] != 0x03)
			return nullptr;
		auto rs = Secp256k1::PubKey();
		try {
			rs = Secp256k1::PubKey::from_buffer(&rs_data[0]);
		} catch (Secp256k1::InvalidPubKey const&) {
			return nullptr;
		}
		/* 5. `h = SHA-256(h || c)` */
		handshake.mix_h(&c[0], c.size());
		/* 6. `se = ECDH(e.priv, rs)` */
		auto se = Secp256k1::ecdh(e.priv(), rs);
		/* 7. `ck, temp_k3 = HKDF(ck, se)` */
		auto temp_k3 = handshake.mix_ck(se);
		/* 8. `p = decryptWithAD(temp_k3, 0, h, t)` */
		auto p = Noise::Detail::Aead::decrypt( temp_k3
						     , 0
						     , vectorize_secret(handshake.get_h())
						     , t
						     );
		if (!p)
			return nullptr;
		assert(*p == nulldata);
		/* Steps 9 and 10 are effectively done by get_encryptor.  */

		act = -1;

		return Util::make_unique<Secp256k1::PubKey>(std::move(rs));
	}

	Noise::Encryptor get_encryptor() {
		if (act != -1)
			throw std::logic_error("Noise::Responder: misuse of get_encryptor.");

		act = 0;

		/* 9. `rk, sk = HKDF(ck, zero)` */
		/* 10. `rn = 0, sn = 0` */
		auto split = handshake.split_ck();
		auto& rk = split.first;
		auto& sk = split.second;

		auto r = Noise::Detail::CipherState(rk); /* sets rn = 0. */
		auto s = Noise::Detail::CipherState(sk); /* sets sn = 0. */

		return Noise::Encryptor( std::move(r)
				       , std::move(s)
				       , handshake.get_ck()
				       );
	}

};

Responder::Responder( Secp256k1::KeyPair const& s
		    , Secp256k1::KeyPair const& e
		    , std::string const& prologue
		    , std::string const& protocol_name
		    ) : pimpl(Util::make_unique<Impl>( s, e
						     , prologue, protocol_name
						     ))
		      { }
Responder::Responder(Responder&& o) : pimpl(std::move(o.pimpl)) { }
Responder::~Responder() { }

std::unique_ptr<std::vector<std::uint8_t>>
Responder::act1_and_2(std::vector<std::uint8_t> const& act1) {
	if (!pimpl)
		throw std::logic_error("Noise::Responder: misuse of act1_and_2().");
	auto ret = pimpl->act1_and_2(act1);
	if (!ret)
		pimpl = nullptr;
	return ret;
}

std::unique_ptr<Secp256k1::PubKey>
Responder::act3(std::vector<std::uint8_t> const& act3) {
	if (!pimpl)
		throw std::logic_error("Noise::Responder: misuse of act3().");
	auto ret = pimpl->act3(act3);
	if (!ret)
		pimpl = nullptr;
	return ret;
}

Noise::Encryptor Responder::get_encryptor() {
	if (!pimpl)
		throw std::logic_error("Noise::Responder: misuse of get_encryptor().");
	auto ret = pimpl->get_encryptor();
	pimpl = nullptr;
	return ret;
}

}

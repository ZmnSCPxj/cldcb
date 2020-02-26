#include<assert.h>
#include<stdexcept>
#include"Noise/Detail/hkdf2.hpp"
#include"Noise/Encryptor.hpp"
#include"Util/make_unique.hpp"

namespace {

auto const nulldata = std::vector<std::uint8_t>();

void rekey(Crypto::Secret& ck, Noise::Detail::CipherState& k) {
	assert(k.get_key());
	auto ckk = Noise::Detail::hkdf2(ck, *k.get_key());
	ck = ckk.first;
	/* This also resets nonce n to 0.  */
	k.initialize_key(ckk.second);
}

}

namespace Noise {

void Encryptor::initial_checks() {
	if ( r.get_nonce() != 0 || s.get_nonce() != 0
	  || !r.get_key() || !s.get_key()
	   )
		throw std::logic_error("Noise::Detail::Encryptor(): r and s cipherstates must have nonce of 0 and must have key.");
}

std::unique_ptr<std::uint16_t>
Encryptor::decrypt_length(std::vector<std::uint8_t> const& l_ciphertext) {
	if (r_state != 0)
		throw std::logic_error("Noise::Detail::Encryptor: Incorrect use of decrypt_length.");
	if (l_ciphertext.size() != 18)
		throw std::logic_error("Noise::Detail::Encryptor: Encrypted length must be size 18 bytes.");

	r_state = 1;

	auto p_l = r.decrypt_with_ad(nulldata, l_ciphertext);
	if (!p_l)
		return nullptr;

	auto& l = *p_l;
	assert(l.size() == 2);
	return Util::make_unique<std::uint16_t>( (std::uint16_t(l[0]) << 8)
					       | (std::uint16_t(l[1]) << 0)
					       );
}

std::unique_ptr<std::vector<std::uint8_t>>
Encryptor::decrypt_message(std::vector<std::uint8_t> const& m_ciphertext) {
	if (r_state != 1)
		throw std::logic_error("Noise::Detail::Encryptor: Incorrect use of decrypt_message.");

	r_state = 0;

	auto p_m = r.decrypt_with_ad(nulldata, m_ciphertext);

	if (r.get_nonce() == 1000)
		rekey(rck, r);

	return p_m;
}

std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>
Encryptor::encrypt_message(std::vector<std::uint8_t> const& plaintext) {
	auto m_size = plaintext.size();
	if (m_size > 65535)
		throw std::logic_error("Noise::Detail::Encryptor: Too long message for encrypt_message.");

	auto l = std::vector<std::uint8_t>(2);
	l[0] = (m_size >> 8) & 0xFF;
	l[1] = (m_size >> 0) & 0xFF;

	auto l_ciphertext = s.encrypt_with_ad(nulldata, l);
	auto m_ciphertext = s.encrypt_with_ad(nulldata, plaintext);

	if (s.get_nonce() == 1000)
		rekey(sck, s);

	return std::make_pair(std::move(l_ciphertext), std::move(m_ciphertext));
}

}

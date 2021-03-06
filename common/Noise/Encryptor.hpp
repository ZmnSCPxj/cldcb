#ifndef CLDCB_COMMON_NOISE_ENCRYPTOR_HPP
#define CLDCB_COMMON_NOISE_ENCRYPTOR_HPP

#include<cstdint>
#include<memory>
#include<utility>
#include<vector>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/CipherState.hpp"

namespace Noise {

/* Performs encryption and decryption for
 * the bidirectional message tunnel.
 */
class Encryptor {
private:
	Detail::CipherState r;
	Crypto::Secret rck;
	int r_state;
	Detail::CipherState s;
	Crypto::Secret sck;

	void initial_checks();

public:
	Encryptor() =delete;

	Encryptor(Encryptor const&) =default;
	Encryptor(Encryptor&&) =default;
	Encryptor& operator=(Encryptor const&) =default;
	Encryptor& operator=(Encryptor&&) =default;

	Encryptor( Detail::CipherState&& r_
		 , Detail::CipherState&& s_
		 , Crypto::Secret const& ck_
		 ) : r(std::move(r_))
		   , rck(ck_)
		   , r_state(0)
		   , s(std::move(s_))
		   , sck(ck_)
		   { initial_checks(); }

	Encryptor( Crypto::Secret const& rk
		 , Crypto::Secret const& sk
		 , Crypto::Secret const& ck
		 ) : r(rk)
		   , rck(ck)
		   , r_state(0)
		   , s(sk)
		   , sck(ck)
		   { initial_checks(); }

	/* Decrypt the length of the succeeding message.
	 * The encrypted length must be exactly 18 bytes.
	 * Afterwards, you should read the length + 16 bytes from
	 * the socket.
	 * Returns nullptr on decryption failure
	 */
	std::unique_ptr<std::uint16_t>
	decrypt_length(std::vector<std::uint8_t> const &l_ciphertext);

	static auto constexpr l_ciphertext_size = 18;

	/* Decrypt the actual message.
	 * Returns nullptr on decryption failure
	 */
	std::unique_ptr<std::vector<std::uint8_t>>
	decrypt_message(std::vector<std::uint8_t> const& m_ciphertext);

	static auto constexpr m_ciphertext_additional_size = 16;

	/* Encrypt the message.
	 * The first returned value is the encrypted length,
	 * the second is the encrypted message.
	 * Afterwards, you should send the encrypted length
	 * followed by the encrypted message.
	 */
	std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>
	encrypt_message(std::vector<std::uint8_t> const& plaintext);

	/* Get the current rk and sk; primarily used for unit test.  */
	Crypto::Secret const& get_rk() const { return *r.get_key(); }
	Crypto::Secret const& get_sk() const { return *s.get_key(); }
};

}

#endif /* CLDCB_COMMON_NOISE_ENCRYPTOR_HPP */

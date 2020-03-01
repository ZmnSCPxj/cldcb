#ifndef CLDCB_COMMON_NOISE_RESPONDER_HPP
#define CLDCB_COMMON_NOISE_RESPONDER_HPP

#include<cstdint>
#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace Noise { class Encryptor; }
namespace Secp256k1 { class KeyPair; }
namespace Secp256k1 { class PubKey; }

namespace Noise {

class Responder {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Responder() =delete;
	Responder(Responder&&);
	Responder& operator=(Responder&& o) {
		auto tmp = std::move(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}
	~Responder();

	/* FIXME: We do not actually need access to the
	 * static private key;
	 * we just need an ECDH operation with a public
	 * key.
	 */
	/* NOTE: The responder keeps a reference to our static key s,
	 * but copies our ephemeral key.
	 */
	Responder( Secp256k1::KeyPair const& s /* Our static key.  */
		 , Secp256k1::KeyPair const& e /* Our ephemeral key.  Should be random.  */
		 , std::string const& prologue = "lightning"
		 , std::string const& protocol_name = "Noise_XK_secp256k1_ChaChaPoly_SHA256"
		 );

	/* FIXME: In principle this should be 3 objects.
	 * See Noise::Initiator for discussion.  */

	/* Accept the Act 1 message and generate the Act 2
	 * message.
	 * The given argument must be 50 bytes from the
	 * initiator.
	 * Return nullptr if Act 1 decryption fails
	 * and we should abort.
	 * Otherwise returns the Act 3 message.
	 * Preconditions: None of act1_and_2, act3, and
	 * get_encryptor have been called.
	 * Postconditions: only act3 can be called; if
	 * this returned nullptr then none of the
	 * member functions can be called and this
	 * object can only be destructed.
	 */
	std::unique_ptr<std::vector<std::uint8_t>>
	act1_and_2(std::vector<std::uint8_t> const& act1);

	static constexpr auto act1_size = std::size_t(50);

	/* Accept the Act 3 message and return the initiator
	 * public key.
	 * The given argument must be 66 bytes from the
	 * initiator.
	 * Return nullptr if Act 3 decryption fails
	 * and we should abort.
	 * Otherwise returns the initiator publiv key.
	 * Preconditions: act1_and_2 has been called,
	 * and neither act3 nor get_encryptor have
	 * been called.
	 * Postconditions: only get_encryptor can be
	 * called; if this returned nullptr then none
	 * of the member functions can be called and
	 * this object can only be destructed.
	 */
	std::unique_ptr<Secp256k1::PubKey>
	act3(std::vector<std::uint8_t> const& act3);

	static constexpr auto act3_size = std::size_t(66);

	/* Construct the encryptor that will be used
	 * in future message communications.
	 * Precondition: act1_and_2 and act3 have been
	 * called once and returned successfully,
	 * and get_encryptor has not been called.
	 * Postcondition: none of the functions can be
	 * called and this object can only be destructed.
	 */
	Noise::Encryptor get_encryptor();
};

}

#endif /* CLDCB_COMMON_NOISE_RESPONDER_HPP */

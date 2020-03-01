#ifndef CLDCB_COMMON_NOISE_INITIATOR_HPP
#define CLDCB_COMMON_NOISE_INITIATOR_HPP

#include<cstdint>
#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace Noise { class Encryptor; }
namespace Secp256k1 { class KeyPair; }
namespace Secp256k1 { class PubKey; }

namespace Noise {

/* A state machine for the initiator handshake.
 *
 * The handshake is composed of these messages:
 *
 * 1.  Act 1 message, 50 bytes.
 * 2.  Act 2 message, 50 bytes.
 * 3.  Act 3 message, 66 bytes.
 *
 * The Initiator sends Act 1, receives Act 2,
 * and sends Act 3.
 * Thus, to use the below handshake class correctly.
 * you must initialize it properly, call act1() to
 * get the Act 1 message and send it to the responder,
 * wait for 50 bytes from the responder, then
 * call act2_and_3 with the Act 2 message from the
 * responder and get the Act 3 message, then send
 * the Act 3 message to the responder and then
 * use get_encryptor() to get the encryptor for the
 * bidirectional message tunnel.
 */
class Initiator {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Initiator() =delete;
	Initiator(Initiator const&) =delete;
	Initiator(Initiator&&);
	Initiator& operator=(Initiator&& o) {
		auto tmp = std::move(o);
		tmp.pimpl.swap(pimpl);
		return *this;
	}
	~Initiator();

	/* FIXME: We do not actually need access to the
	 * static private key;
	 * we just need an ECDH operation with a public
	 * key.
	 */
	/* NOTE: The initiator keeps a reference to our static key s,
	 * but copies our ephemeral key.
	 */
	Initiator( Secp256k1::KeyPair const& s /* Our static key.  */
		 , Secp256k1::PubKey const& rs /* The responder static key.  */
		 , Secp256k1::KeyPair const& e /* Our ephemeral key.  Should be random.  */
		 , std::string const& prologue = "lightning"
		 , std::string const& protocol_name = "Noise_XK_secp256k1_ChaChaPoly_SHA256"
		 );

	/* FIXME: In principle, this should be 3 objects:
	 *
	 * * An Act 1 object.
	 * * An object awaiting for Act 2 and returning the
	 *   Act 3 message.
	 * * A handshake-completion object that returns
	 *   the encryptor.
	 *
	 * This lets us dispense with all the precondition
	 * and postcondition nonsense, and ensures
	 * compile-time checking of correct state machine
	 * usage.
	 */

	/* Generate the Act 1 message.
	 * Precondition: none of act1, act2_and_3, and
	 * get_encryptor have been called.
	 * Postcondition: only act2_and_3 can be called.
	 */
	std::vector<std::uint8_t> act1();
	/* Accept the Act 2 message and generate the
	 * Act 3 message.
	 * The given argument must be 50 bytes from the
	 * responder.
	 * Return nullptr if Act 2 decryption fails
	 * and we should abort.
	 * Otherwise returns the Act 3 message.
	 * Preconditions: only act1 has been called
	 * once, and act2_and_3 and get_encryptor have
	 * not been called.
	 * Postconditions: only get_encryptor can be
	 * called; if this returned nullptr then none
	 * of the member functions can be called and
	 * this object can only be destructed.
	 */
	std::unique_ptr<std::vector<std::uint8_t>>
	act2_and_3(std::vector<std::uint8_t> const& act2);
	/* Construct the encryptor that will be used in
	 * future message communications.
	 * Precondition: act1 and act2_and_3 have been
	 * called once (and act2_and_3 did not fail),
	 * and get_encryptor has not been called.
	 * Postcondition: none of the functions can be
	 * called and this object can only be destructed.
	 */
	Noise::Encryptor get_encryptor();
};

}

#endif /* CLDCB_COMMON_NOISE_INITIATOR_HPP */

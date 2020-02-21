#ifndef CLDCB_COMMON_SECP256K1_ECDH_HPP
#define CLDCB_COMMON_SECP256K1_ECDH_HPP

namespace Crypto { class Secret; }
namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }

namespace Secp256k1 {

/*
 * Elliptic-curve Diffie-Hellman, a key agreement scheme
 * that requires that you and your counterparty send
 * public keys to each other, and for you to determine
 * the agreed-upon shared key using your own private key
 * and the public key you received from the counterparty.
 *
 * The exact details of the standard used here is the
 * standard as used in Lightning and elsewhere in the
 * Bitcoin world: the privkey is multiplied with the
 * pubkey, the resulting pubkey is represented as a
 * compressed DER public key, then the DER encoding is
 * hashed with SHA256, resulting in a shared secret.
 *
 * Note that most other non-Bitcoin standards have a
 * slightly different sense of "ECDH":
 * the privkey is multiplied with the pubkey, the X
 * coordinate of the resulting pubkey is used as the
 * resulting private key.
 * This is mildly undesirable as the X coordinate has
 * less than 256 bits of entropy.
 */
Crypto::Secret ecdh(PrivKey, PubKey);

}

#endif /* CLDCB_COMMON_SECP256K1_ECDH_HPP */

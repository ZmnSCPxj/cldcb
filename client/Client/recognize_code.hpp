#ifndef CLDCB_CLIENT_CLIENT_RECOGNICE_CODE_HPP
#define CLDCB_CLIENT_CLIENT_RECOGNICE_CODE_HPP

namespace Secp256k1 { class PrivKey; }
namespace Secp256k1 { class PubKey; }
namespace Secp256k1 { class Signature; }

namespace Client {

/* Check if the given signature is a recognizable code,
 * given our setup.
 *
 * A recognition code is simply a signature whose message
 * we can derive from our secret data, but which an
 * arbitrary third party cannot derive.
 * This prevents third parties from deriving our public
 * key.
 */
bool recognize_code( Secp256k1::PubKey const& signer
		   , Secp256k1::Signature const& recognition_code
		   , Secp256k1::PrivKey const& ecdh_sk
		   , Secp256k1::PubKey const& ecdh_pk
		   );

/* Generate the message to be signed.  */
std::string recognition_code_message( Secp256k1::PrivKey const& ecdh_sk
				    , Secp256k1::PubKey const& ecdh_pk
				    );

}

#endif /* CLDCB_CLIENT_CLIENT_RECOGNICE_CODE_HPP */

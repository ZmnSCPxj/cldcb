#ifndef CLDCB_COMMON_NOISE_DETAIL_AEAD_HPP
#define CLDCB_COMMON_NOISE_DETAIL_AEAD_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Crypto { class Secret; }

/* Implements CHACHA20POL1305 AEAD.
 * The nonce is encoded as 4 0x00 bytes, followed by
 * the **little-endian** encoding of the given 64-bit
 * nonce.
 */
namespace Noise { namespace Detail { namespace Aead {

std::vector<std::uint8_t>
encrypt( Crypto::Secret const& key
       , std::uint64_t nonce
       , std::vector<std::uint8_t> const& ad
       , std::vector<std::uint8_t> const& plaintext
       );
/* Return nullptr if decryption fails.  */
std::unique_ptr<std::vector<std::uint8_t>>
decrypt( Crypto::Secret const& key
       , std::uint64_t nonce
       , std::vector<std::uint8_t> const& ad
       , std::vector<std::uint8_t> const& ciphertext
       );

}}}

#endif /* CLDCB_COMMON_NOISE_DETAIL_AEAD_HPP */

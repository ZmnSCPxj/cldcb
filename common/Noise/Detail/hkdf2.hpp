#ifndef CLDCB_COMMON_NOISE_DETAIL_HKDF_HPP
#define CLDCB_COMMON_NOISE_DETAIL_HKDF_HPP

#include<utility>

namespace Crypto { class Secret; }

namespace Noise { namespace Detail {

/* An HKDF call that gets two 256-bit secrets
 * and returns two 256-bit secrets.  */
std::pair<Crypto::Secret, Crypto::Secret>
hkdf2(Crypto::Secret const&, Crypto::Secret const&);

/* An HKDF call that gets a 256-bit secret and
 * uses an zero-length second argument, and
 * returns two 256-bit secrets.
 */
std::pair<Crypto::Secret, Crypto::Secret>
hkdf2_zero(Crypto::Secret const&);

}}

#endif /* CLDCB_COMMON_NOISE_DETAIL_HKDF_HPP */

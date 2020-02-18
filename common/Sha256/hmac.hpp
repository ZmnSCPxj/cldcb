#ifndef CLDCB_COMMON_SHA256_HMAC_HPP
#define CLDCB_COMMON_SHA256_HMAC_HPP

#include<cstdlib>

namespace Sha256 { class Hash; }

namespace Sha256 {

Hash hmac( void const *key, std::size_t keylen
	 , void const *data, std::size_t datalen
	 );

}

#endif /* CLDCB_COMMON_SHA256_HMAC_HPP */

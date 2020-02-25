#ifndef CLDCB_COMMON_SHA256_HKDF_HPP
#define CLDCB_COMMON_SHA256_HKDF_HPP

#include<cstdlib>

namespace Sha256 {

void hkdf( void* okm, std::size_t okmlen
	 , void const* salt, std::size_t saltlen
	 , void const* ikm, std::size_t ikmlen
	 );

}

#endif /* CLDCB_COMMON_SHA256_HKDF_HPP */

#ifndef CLDCB_COMMON_SHA256_FUN_HPP
#define CLDCB_COMMON_SHA256_FUN_HPP

#include<cstdlib>

namespace Sha256 { class Hash; }

namespace Sha256 {

Hash fun(void const *, std::size_t len);

}

#endif /* CLDCB_COMMON_SHA256_FUN_HPP */

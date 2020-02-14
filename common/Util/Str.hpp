#ifndef CLDCB_COMMON_UTIL_STR_HPP
#define CLDCB_COMMON_UTIL_STR_HPP

/*
 * Minor string utilities.
 */

#include<cstdint>
#include<string>

namespace Util {
namespace Str {

/* Outputs a two-digit hex string of the given byte.  */
std::string hexbyte(std::uint8_t);

}
}

#endif /* CLDCB_COMMON_UTIL_STR_HPP */

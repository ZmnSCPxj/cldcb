#ifndef CLDCB_COMMON_UTIL_STR_HPP
#define CLDCB_COMMON_UTIL_STR_HPP

/*
 * Minor string utilities.
 */

#include<cstdint>
#include<stdexcept>
#include<string>
#include<vector>

namespace Util {
namespace Str {

/* Outputs a two-digit hex string of the given byte.  */
std::string hexbyte(std::uint8_t);

/* Creates a buffer from the given hex string.  */
/* FIXME: use a backtrace-extracting exception. */
struct HexParseFailure : public std::runtime_error {
	HexParseFailure(std::string msg)
		: std::runtime_error("hexread: " + msg) { }
};
std::vector<std::uint8_t> hexread(std::string const&);

}
}

#endif /* CLDCB_COMMON_UTIL_STR_HPP */

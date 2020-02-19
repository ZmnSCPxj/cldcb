#ifndef CLDCB_COMMON_JSMN_JSONIFY_STRING_HPP
#define CLDCB_COMMON_JSMN_JSONIFY_STRING_HPP

#include<string>

namespace Jsmn {

/* Transform an ordinary C++ string to a string which,
 * when printed, is a valid JSON string.
 */
std::string jsonify_string(std::string const& s);

}

#endif /* CLDCB_COMMON_JSMN_JSONIFY_STRING_HPP */

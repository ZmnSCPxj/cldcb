#ifndef CLDCB_CLIENT_PLUGIN_OPTFILE_VALIDATE_KEYS_HPP
#define CLDCB_CLIENT_PLUGIN_OPTFILE_VALIDATE_KEYS_HPP

#include<map>
#include<string>

namespace Plugin { namespace OptFile {

/* Returns an error message if validation error,
 * returns an empty string if valid.
 */
std::string validate_keys(std::map<std::string, std::string> const& options);

}}

#endif /* CLDCB_CLIENT_PLUGIN_OPTFILE_VALIDATE_KEYS_HPP */

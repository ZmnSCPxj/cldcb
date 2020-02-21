#ifndef CLDCB_CLIENT_PLUGIN_OPTFILE_PARSE_HPP
#define CLDCB_CLIENT_PLUGIN_OPTFILE_PARSE_HPP

#include<istream>
#include<map>
#include<string>

namespace Plugin { namespace OptFile {

/* Parses the option file naively, extracting key-value pairs.
 * This does not validate that the options are correctly-formatted,
 * that keys are correct, etc.
 */
std::map<std::string, std::string> parse(std::istream&);

}}

#endif /* CLDCB_CLIENT_PLUGIN_OPTFILE_PARSE_HPP */

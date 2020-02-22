#ifndef CLDCB_CLIENT_PLUGIN_OPTFILE_TRANSLATE_SETUP_HPP
#define CLDCB_CLIENT_PLUGIN_OPTFILE_TRANSLATE_SETUP_HPP

#include<map>
#include<string>

namespace Plugin { class Setup; }

namespace Plugin { namespace OptFile {

/* Return a string describing the error, or
 * "" if no error.
 */
std::string translate_setup( Plugin::Setup&
			   , std::map<std::string, std::string> const&
			   );

}}

#endif /* CLDCB_CLIENT_PLUGIN_OPTFILE_TRANSLATE_SETUP_HPP */

#ifndef CLDCB_CLIENT_PLUGIN_OPTFILE_LOAD_HPP
#define CLDCB_CLIENT_PLUGIN_OPTFILE_LOAD_HPP

#include<memory>

namespace Plugin { class Setup; }

namespace Plugin { namespace OptFile {

/* Prints any failures on std::cerr, returns a
 * non-null pointer on failure.
 * Returns null pointer if we should continue and
 * the given setup has been loaded.
 */
std::unique_ptr<int> load(Plugin::Setup&, std::string filename);

}}

#endif /* CLDCB_CLIENT_PLUGIN_OPTFILE_LOAD_HPP */

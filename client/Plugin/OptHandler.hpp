#ifndef CLDCB_CLIENT_PLUGIN_OPTHANDLER_HPP
#define CLDCB_CLIENT_PLUGIN_OPTHANDLER_HPP

#include<memory>
#include<string>

namespace Plugin {

class OptHandler {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	OptHandler() =delete;
	~OptHandler();
	OptHandler(OptHandler&& o);
	explicit OptHandler(int argc, char** argv);

	/* Return nullptr to continue normal processing.
	 * Return non-null to immediately exit with the
	 * returned int as exit code.
	 */
	std::unique_ptr<int> check_options();

	/* Determine the filename of the options file
	 * (plugin wrapper script).
	 * Precondition: check_options() must have been
	 * called and returned nullptr.
	 */
	std::string const& options_filename() const;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_OPTHANDLER_HPP */

#include<errno.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Server/OptionsHandler.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Server {

class OptionsHandler::Impl {
private:
	Util::Logger& logger;
	std::string serverdir;
	std::string pidfile;
	std::string logfile;
	std::uint16_t max_count;
	int port;

	static
	std::string get_home_dir() {
		return std::string(getenv("HOME"));
	}

public:
	explicit
	Impl(Util::Logger& logger_) : logger(logger_) {
		serverdir = get_home_dir() + "/.cldcb-server";
		pidfile = "cldcb-server.pid";
		logfile = "debug.log";
		max_count = std::uint16_t(19999);
		port = 29735;
	}

	std::unique_ptr<int>
	handle(std::vector<std::string>& params) {

		/* TODO: Actually handle options.  */

		/* Enter the serverdir.
		 * If not exist, create.
		 */
		for (;;) {
			auto cdres = chdir(serverdir.c_str());
			if (cdres < 0 && errno == ENOENT) {
				auto mdres = mkdir( serverdir.c_str()
						  , 0700
						  );
				if (mdres < 0) {
					auto my_errno = errno;
					logger.BROKEN( "%s not found and "
						       "could not be created: "
						       "%s"
						     , serverdir.c_str()
						     , strerror(my_errno)
						     );
					return Util::make_unique<int>(1);
				}
				continue;
			}
			/* Success.  */
			if (cdres == 0)
				break;

			auto my_errno = errno;
			logger.BROKEN( "Cannot chdir to %s: %s"
				     , serverdir.c_str()
				     , strerror(my_errno)
				     );
			return Util::make_unique<int>(1);
		}

		return nullptr;
	}

	std::string const& get_pidfile() const { return pidfile; }
	std::string const& get_logfile() const { return logfile; }
	std::uint16_t get_max_count() const { return max_count; }
	int get_port() const { return port; }
};

OptionsHandler::OptionsHandler(Util::Logger& logger)
	: pimpl(Util::make_unique<Impl>(logger))
	{ }
OptionsHandler::OptionsHandler(OptionsHandler&& o)
	: pimpl(std::move(o.pimpl)) { }

OptionsHandler::~OptionsHandler() { }

std::unique_ptr<int>
OptionsHandler::handle(std::vector<std::string>& params) {
	return pimpl->handle(params);
}

std::string const&
OptionsHandler::pidfile() const {
	return pimpl->get_pidfile();
}
std::string const&
OptionsHandler::logfile() const {
	return pimpl->get_logfile();
}
std::uint16_t
OptionsHandler::max_count() const {
	return pimpl->get_max_count();
}
int
OptionsHandler::port() const {
	return pimpl->get_port();
}

}

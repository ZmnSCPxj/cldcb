#include<errno.h>
#include<iostream>
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
	std::string helpline;
	Supported supp;

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

	static
	bool is_opt(std::string const& param) {
		return (param.length() > 0) && (param[0] == '-');
	}
	/* Given is_equals_opt(param, "--foo"), checks if param
	 * starts with "--foo=".
	 */
	static
	bool is_equals_opt( std::string const& param
			  , std::string const& opt
			  ) {
		if (opt.length() > param.length())
			return false;
		auto preparam = param.substr(0, opt.length() + 1);
		return preparam == (opt + "=");
	}
	static
	std::string get_equals_opt(std::string const& param) {
		auto it = param.find('=');
		if (it == std::string::npos)
			return "";
		return std::string(param.begin() + it + 1, param.end());
	}

	/* Remove this option from the params, return the equivalenti
	 * iterator afterwards.
	 */
	static void
	shift( std::vector<std::string>& params
	     , std::vector<std::string>::iterator& it
	     ) {
		auto pos = it - params.begin();
		params.erase(it);
		it = params.begin() + pos;
	}

	void do_help() const {
		std::cout << "Usage: " << helpline << std::endl
			  << std::endl
			  << "Supported options of this method:" << std::endl
			  << "  -h, --help          Display this help." << std::endl
			  << "  --server-dir=<dir>  Directory to keep everything in." << std::endl
			  << "                       (clients, secret, server_id, archive files...)" << std::endl
			  << "  --pidfile=<file>    File to contain PID of running daemon." << std::endl
			  << "                       (relative to server-dir)" << std::endl
			   ;
		if (supp == All) {
			std::cout
			  << "  --logfile=<file>    File to put logs in (relative to server-dir)." << std::endl
			  << "  --maxcount=<num>    Maximum number of increments before forcing reupload." << std::endl
			  << "  --port=<num>        Port to listen on." << std::endl
			   ;
		}
		std::cout << std::endl
			  << "cldcb-server is Free Software WITHOUT ANY WARRANTY." << std::endl
			  << std::endl
			   ;
	}

	struct BadArg {};

	bool param_string( std::vector<std::string>& params
			 , std::vector<std::string>::iterator& pt
			 , std::string option
			 , std::string& target
			 ) {
		if (pt == params.end())
			return true;
		if (*pt == option) {
			shift(params, pt);
			if (pt == params.end()) {
				logger.BROKEN( "Missing arg for: %s"
					     , option.c_str()
					     );
				throw BadArg();
			}
			target = *pt;
			shift(params, pt);
			return true;
		}
		if (is_equals_opt(*pt, option)) {
			target = get_equals_opt(*pt);
			shift(params, pt);
			return true;
		}
		return false;
	}
	bool param_int( std::vector<std::string>& params
		      , std::vector<std::string>::iterator& pt
		      , std::string option
		      , int& target
		      ) {
		auto tmp = std::string();
		auto ret = param_string(params, pt, option, tmp);
		if (ret) {
			auto pos = std::size_t(0);
			auto tmp_int = int();
			try {
				tmp_int = std::stoi(tmp, &pos);
			} catch (std::runtime_error const&) {
				pos = tmp.length() - 1;
			}
			if (pos != tmp.length()) {
				logger.BROKEN( "Expected numeric argument "
					       "for %s, got %s."
					     , option.c_str()
					     , tmp.c_str()
					     );
				throw BadArg();
			}

			target = tmp_int;
		}
		return ret;
	}

public:
	explicit
	Impl( Util::Logger& logger_
	    , std::string helpline_
	    , Supported supp_
	    ) : logger(logger_)
	      , helpline(std::move(helpline_))
	      , supp(supp_)
	      {
		serverdir = get_home_dir() + "/.cldcb-server";
		pidfile = "cldcb-server.pid";
		logfile = "debug.log";
		max_count = std::uint16_t(19999);
		port = 29735;
	}

	std::unique_ptr<int>
	handle(std::vector<std::string>& params) {

		auto pt = params.begin();
		while (pt != params.end()) {
			if (!is_opt(*pt)) {
				/* Skip non-options.  */
				++pt;
				continue;
			}

			if (*pt == "--") {
				/* Shift, then leave the rest of the
				 * parameters alone.
				 */
				shift(params, pt);
				break;
			}

			if (*pt == "--help" || *pt == "-h") {
				do_help();
				return Util::make_unique<int>(0);
			}

			try {
				auto max_count_int = int();

				if (param_string( params, pt
						, "--server-dir", serverdir
						))
					continue;
				if (param_string( params, pt
						, "--pidfile", pidfile
						))
					continue;

				if (supp != All)
					goto skip;
				if (param_string( params, pt
						, "--logfile", logfile
						))
					continue;
				if (param_int( params, pt
					     , "--port", port
					     ))
					continue;
				if (param_int( params, pt
					     , "--maxcount", max_count_int
					     )) {
					if ( max_count_int > 65536
					  || max_count_int <= 0
					   ) {
						logger.BROKEN( "--maxcount "
							       "out of range:"
							       " %d"
							     , max_count_int
							     );
						throw BadArg();
					}
					max_count = std::uint16_t(
						max_count_int - 1
					);
				}
			skip:
				;
			} catch (BadArg const& e) {
				return Util::make_unique<int>(1);
			}

			/* Unknown option.  */
			logger.BROKEN( "Unrecognized option: %s"
				     , pt->c_str()
				     );
			return Util::make_unique<int>(1);
		}

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

OptionsHandler::OptionsHandler( Util::Logger& logger
			      , std::string helpline
			      , Supported supp
			      )
	: pimpl(Util::make_unique<Impl>(logger, std::move(helpline), supp))
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

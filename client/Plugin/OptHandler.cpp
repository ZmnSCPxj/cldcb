#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<iostream>
#include<stdexcept>
#include"Plugin/OptHandler.hpp"
#include"Util/make_unique.hpp"

namespace Plugin {

class OptHandler::Impl {
private:
	int argc;
	char** argv;

	std::string options_filename_str;

	std::unique_ptr<int> arg_req() {
		std::cerr << argv[0] << ": Argument required."
			  << std::endl
			  ;
		return Util::make_unique<int>(1);
	}
	std::unique_ptr<int> too_many_arg() {
		std::cerr << argv[0] << ": Too many arguments."
			  << std::endl
			  ;
		return Util::make_unique<int>(1);
	}

public:
	Impl() =delete;
	Impl(Impl const&) =delete;

	explicit Impl(int argc_, char** argv_)
		: argc(argc_), argv(argv_) { }

	std::unique_ptr<int> check_options() {
		if (argc <= 1)
			return arg_req();
		if ( argv[1] == std::string("--version")
		  || argv[1] == std::string("-v")
		   ) {
			std::cout << PACKAGE_STRING << std::endl;
			return Util::make_unique<int>(0);
		}
		if ( argv[1] == std::string("--help")
		  || argv[1] == std::string("-h")
		   ) {
			std::cout << "Usage: cldcb-plugin optionsfile" << std::endl
				  << std::endl
				  << "A plugin for continuous channel backups for C-Lightning." << std::endl
				  << "Use cldcb-client to manage." << std::endl
				  << std::endl
				  << "Supported options:" << std::endl
				  << " -h, --help      Display this help." << std::endl
				  << " -v, --version   Display version." << std::endl
				  << std::endl
				  << "cldcb-plugin is Free Software WITHOUT ANY WARRANTY." << std::endl
				  << std::endl
				  ;
			return Util::make_unique<int>(0);
		}
		if ( argv[1][0] == '-'
		  && argv[1] != std::string("--")
		   ) {
			std::cerr << argv[0] << ": Unrecognized option: "
				  << argv[1] << std::endl
				  ;
			return Util::make_unique<int>(1);
		}
		if (argv[1] == std::string("--")) {
			if (argc < 3)
				return arg_req();
			if (argc > 3)
				return too_many_arg();
			options_filename_str = argv[2];
			return nullptr;
		}
		if (argc > 2)
			return too_many_arg();
		options_filename_str = argv[1];
		return nullptr;
	}

	std::string const& options_filename() const {
		return options_filename_str;
	}
};

OptHandler::~OptHandler() { }

OptHandler::OptHandler(int argc, char** argv)
	: pimpl(Util::make_unique<Impl>(argc, argv)) { }

OptHandler::OptHandler(OptHandler&& o) {
	o.pimpl.swap(pimpl);
}

std::unique_ptr<int> OptHandler::check_options() {
	if (!pimpl)
		throw std::logic_error("Use of Plugin::OptHandler after being moved from.");
	return pimpl->check_options();
}

std::string const& OptHandler::options_filename() const {
	if (!pimpl)
		throw std::logic_error("Use of Plugin::OptHandler after being moved from.");
	return pimpl->options_filename();
}

}

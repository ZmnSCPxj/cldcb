#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<map>
#include"Main.hpp"
#include"Util/make_unique.hpp"

namespace {

std::vector<std::string> make_params(int argc, char** argv) {
	auto ret = std::vector<std::string>(argc);
	for (auto i = int(0); i < argc; ++i)
		ret[i] = argv[i];
	return ret;
}

}

class Main::Impl {
private:
	std::string program_name;
	std::map< std::string
		, std::function<int (std::vector<std::string>)>
		> methods;

	void do_help(std::ostream& cout) {
		cout << "Usage: " << program_name << " <method> [params]" << std::endl
		     << std::endl
		     << "Supported options outside of methods:" << std::endl
		     << "  -h, --help     Display this help." << std::endl
		     << "  -v, --version  Display version." << std::endl
		     << std::endl
		      ;
		if (!methods.empty())
			cout << "Supported methods:" << std::endl;
		for (auto& method : methods) {
			cout << "  " << method.first << std::endl;
		}
		if (!methods.empty())
			cout << std::endl;
		cout << program_name << " is Free Software WITHOUT ANY WARRANTY." << std::endl;
	}

public:
	Impl(std::string const& program_name_)
		: program_name(program_name_)
		, methods()
		{ }

	void add_method( std::string const& method_name
		       , std::function<int (std::vector<std::string>)> method
		       ) {
		methods.insert(std::make_pair(method_name, method));
	}

	int main( int argc
		, char** argv
		, std::ostream& cout
		, std::ostream& cerr
		) {
		if (argc <= 1) {
			if (argc == 1)
				cerr << argv[0];
			else
				cerr << program_name;
			cerr << ": Expecting method." << std::endl;
			return 1;
		}
		if ( argv[1] == std::string("--version")
		  || argv[1] == std::string("-v")
		   ) {
			cout << PACKAGE_STRING << std::endl;
			return 0;
		}
		if ( argv[1] == std::string("--help")
		  || argv[1] == std::string("-h")
		   ) {
			do_help(cout);
			return 0;
		}
		if (argv[1][0] == '-') {
			cerr << argv[0] << ": Unrecognized option: "
			     << argv[1] << std::endl
			      ;
			return 1;
		}

		auto method_name = std::string(argv[1]);
		auto lookup = methods.find(method_name);
		if (lookup == methods.end()) {
			cerr << argv[0] << ": Unrecognized method: "
			     << method_name << std::endl
			      ;
			return 1;
		}
		auto& method = lookup->second;
		auto params = make_params(argc - 2, argv + 2);

		return method(std::move(params));
	}
};

Main::Main(std::string const& program_name)
	: pimpl(Util::make_unique<Impl>(program_name)) { }
Main::~Main() { }
Main::Main(Main&& o) : pimpl(std::move(o.pimpl)) { }

Main& Main::add_method( std::string const& method_name
		      , std::function<int (std::vector<std::string>)> method
		      ) {
	if (pimpl)
		pimpl->add_method(method_name, method);
	return *this;
}
int Main::main( int argc, char** argv
	      , std::ostream& cout
	      , std::ostream& cerr
	      ) {
	if (pimpl)
		return pimpl->main(argc, argv, cout, cerr);
	else {
		cerr << "Invalid use of Main object after being moved from." << std::endl;
		return 1;
	}
}

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<utility>
#include"LD/DbWrite.hpp"
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/Main.hpp"
#include"Plugin/MainLoop.hpp"
#include"Plugin/OptFile/load.hpp"
#include"Plugin/OptHandler.hpp"
#include"Plugin/Setup.hpp"
#include"Util/make_unique.hpp"

namespace {

class NullDbWriteHandler : public Plugin::DbWriteHandler {
public:
	bool handle( LD::DbWrite const&
		   ) override {
		return true;
	}
};

}

namespace Plugin {

class Main::Impl {
private:
	std::istream& stdin;
	LD::Writer writer;
	LD::Logger log;
	OptHandler options;

	Plugin::Setup setup;

public:
	Impl( std::istream& stdin_, std::ostream& stdout_
	    , int argc, char** argv
	    ) : stdin(stdin_)
	      , writer(stdout_)
	      , log(writer)
	      , options(argc, argv)
	      , setup()
	      { }

	int run() {
		auto options_result = options.check_options();
		if (options_result)
			return *options_result;

		auto optload_result = Plugin::OptFile::load( setup
							   , options.options_filename()
							   );
		if (optload_result)
			return *optload_result;

		auto handler = NullDbWriteHandler();
		auto loop = Plugin::MainLoop(stdin, writer, log, handler);

		log.info( "cldcb-plugin %1$s "
			  "is Free Software WITHOUT ANY WARRANTY; "
			  "report bugs to %2$s"
			, PACKAGE_VERSION
			, PACKAGE_BUGREPORT
			);
		return loop.run();
	}
};

Main::Main(Main&& o) : pimpl(std::move(o.pimpl)) { }
Main::~Main() { }

Main::Main( std::istream& stdin, std::ostream& stdout
	  , int argc, char** argv
	  ) : pimpl(Util::make_unique<Impl>(stdin, stdout, argc, argv)) { }

int Main::run() {
	if (pimpl)
		return pimpl->run();
	return 1;
}

}

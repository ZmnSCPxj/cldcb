#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<utility>
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"
#include"Plugin/Main.hpp"
#include"Plugin/MainLoop.hpp"
#include"Util/make_unique.hpp"

namespace Plugin {

class Main::Impl {
private:
	std::istream& stdin;
	LD::Writer writer;
	LD::Logger log;

public:
	Impl( std::istream& stdin_, std::ostream& stdout_
	    , int argc, char** argv
	    ) : stdin(stdin_)
	      , writer(stdout_)
	      , log(writer)
	      { }

	int run() {
		/* TODO: option processing.  */

		auto loop = Plugin::MainLoop(stdin, writer, log);

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
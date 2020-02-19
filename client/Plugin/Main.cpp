#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<utility>
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"
#include"Plugin/Main.hpp"
#include"Util/make_unique.hpp"

namespace Plugin {

class Main::Impl {
private:
	LD::Writer writer;
	LD::Logger log;

public:
	Impl( std::istream& stdin, std::ostream& stdout
	    , int argc, char** argv
	    ) : writer(stdout)
	      , log(writer)
	      { }

	int run() {
		/* TODO: option processing.  */

		log.info( "cldcb-plugin %1$s "
			  "is Free Software WITHOUT ANY WARRANTY; "
			  "report bugs to %2$s"
			, PACKAGE_VERSION
			, PACKAGE_BUGREPORT
			);
		return 0;
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

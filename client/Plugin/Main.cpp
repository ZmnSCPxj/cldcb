#ifdef HAVE_CONFIG_H
#include"config.h"
#endif
#include<utility>
#include"LD/DbWrite.hpp"
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"
#include"Plugin/DbFileReader.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/Main.hpp"
#include"Plugin/MainLoop.hpp"
#include"Plugin/OptFile/load.hpp"
#include"Plugin/OptHandler.hpp"
#include"Plugin/Setup.hpp"
#include"Plugin/ServerIf.hpp"
#include"Plugin/ServerResult.hpp"
#include"Util/make_unique.hpp"

namespace {

class NullServerIf : public Plugin::ServerIf {
public:
	std::future<Plugin::ServerResult>
	send(std::uint32_t, std::vector<std::uint8_t>) override {
		auto p = std::promise<Plugin::ServerResult>();
		p.set_value(Plugin::ServerResult::success());
		return p.get_future();
	}
};
class NullDbFileReader : public Plugin::DbFileReader {
private:
	class NullSession : public Plugin::DbFileReader::Session {
	public:
		std::vector<std::uint8_t> read(unsigned int) override {
			return std::vector<std::uint8_t>();
		}
	};
public:
	std::unique_ptr<Plugin::DbFileReader::Session>
	start() override {
		return Util::make_unique<NullSession>();
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

		auto server = NullServerIf();
		auto db = NullDbFileReader();
		auto handler = Plugin::DbWriteHandler(setup, server, db);
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

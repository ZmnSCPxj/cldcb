#ifndef CLDCB_CLIENT_PLUGIN_MAINLOOP_HPP
#define CLDCB_CLIENT_PLUGIN_MAINLOOP_HPP

#include<istream>

namespace LD { class Writer; }
namespace Util { class Logger; }

namespace Plugin {

class MainLoop {
private:
	std::istream& stdin;
	LD::Writer& writer;
	Util::Logger& log;

public:
	MainLoop() =delete;

	MainLoop( std::istream& stdin_
		, LD::Writer& writer_
		, Util::Logger& log_
		) : stdin(stdin_), writer(writer_), log(log_) { }

	int run();
};

}

#endif /* CLDCB_CLIENT_PLUGIN_MAINLOOP_HPP */

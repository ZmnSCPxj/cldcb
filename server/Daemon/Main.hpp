#ifndef CLDCB_SERVER_DAEMON_MAIN_HPP
#define CLDCB_SERVER_DAEMON_MAIN_HPP

#include<memory>
#include<string>

namespace Util { class Logger; }

namespace Daemon {

class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;
	Main(Main&&) =delete;
	Main(Main const&) =delete;

	Main( Util::Logger& logger
	    , int port
	    , std::string pid_path
	    );
	~Main();

	void run();
};

}

#endif /* CLDCB_SERVER_DAEMON_MAIN_HPP */
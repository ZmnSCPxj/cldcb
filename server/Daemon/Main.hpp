#ifndef CLDCB_SERVER_DAEMON_MAIN_HPP
#define CLDCB_SERVER_DAEMON_MAIN_HPP

#include<functional>
#include<memory>
#include<string>

namespace Daemon { class Breaker; }
namespace Daemon { class ClientList; }
namespace Daemon { class ConnectionLoop; }
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
	    , std::function< std::unique_ptr<Daemon::ConnectionLoop>
				( Util::Logger&
				, Daemon::Breaker&
				, Daemon::ClientList&
				)
			   > looper_constructor
	    );
	~Main();

	void run();
};

}

#endif /* CLDCB_SERVER_DAEMON_MAIN_HPP */

#ifndef CLDCB_SERVER_DAEMON_CONNECTION_HPP
#define CLDCB_SERVER_DAEMON_CONNECTION_HPP

#include<memory>
#include"Net/SocketFd.hpp"

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Util { class Logger; }

namespace Daemon {

class Connection {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;
	std::string const& prologue;
	Net::SocketFd fd;

public:
	Connection() =delete;

	Connection( Util::Logger& logger
		  , Daemon::Breaker& breaker
		  , std::string const& prologue
		  , Net::SocketFd fd
		  );

	/* This function is passed in a shared pointer so that
	 * the resulting Io<int> can keep the Connection object
	 * alive.
	 */
	static
	Ev::Io<int> new_connection(std::shared_ptr<Connection>);
};

}

#endif /* CLDCB_SERVER_DAEMON_CONNECTION_HPP */

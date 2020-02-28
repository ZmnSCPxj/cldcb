#ifndef CLDCB_SERVER_DAEMON_ACCEPTLOOP_HPP
#define CLDCB_SERVER_DAEMON_ACCEPTLOOP_HPP

#include<functional>
#include<memory>

namespace Ev { template<typename a> class Io; }
namespace Net { class SocketFd; }
namespace Util { class Logger; }

namespace Daemon {

/* Sets up listening over the specified port.
 * Then when accept_loop is started, keeps accepting
 * ports and handing it over to the handler.
 */
class AcceptLoop {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	AcceptLoop() =delete;
	AcceptLoop(AcceptLoop&&) =delete;
	AcceptLoop(AcceptLoop const&) =delete;
	AcceptLoop( int port
		  , Util::Logger& logger
		  , std::function<Ev::Io<int>(Net::SocketFd)> handler
		  );

	~AcceptLoop();

	/* Enter the accept loop, launch concurrent Ev::Io tasks
	 * for each accepted socket using the given handler
	 * function.
	 */
	Ev::Io<int> accept_loop();
};

}

#endif /* CLDCB_SERVER_DAEMON_ACCEPTLOOP_HPP */

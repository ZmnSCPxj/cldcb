#ifndef CLDCB_COMMON_NET_LISTENER_HPP
#define CLDCB_COMMON_NET_LISTENER_HPP

#include<memory>
#include"Net/Fd.hpp"

namespace Net { class SocketFd; }
namespace Util { class Logger; }

namespace Net {

class Listener {
private:
	Net::Fd fd;
	Util::Logger& logger;

public:
	Listener() =delete;
	explicit Listener(int port, Util::Logger& logger);
	Listener(Listener&& o) =default;
	Listener& operator=(Listener&& o) =default;
	Listener(Listener const&) =delete;

	/* Blocks if not ready.  Use select or poll
	 * on get_fd() to wait for multiple events.
	 */
	Net::SocketFd accept();

	int get_fd() const { return fd.get(); }
};

}

#endif /* CLDCB_COMMON_NET_LISTENER_HPP */

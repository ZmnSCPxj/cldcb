#ifndef CLDCB_COMMON_NET_LISTENER_HPP
#define CLDCB_COMMON_NET_LISTENER_HPP

#include<memory>
#include<utility>
#include"Net/Fd.hpp"

namespace Net { class SocketFd; }
namespace Util { class Logger; }

namespace Net {

class Listener {
private:
	Net::Fd fd;
	Util::Logger* plogger;

public:
	Listener() =delete;
	explicit Listener(int port, Util::Logger& logger);
	Listener(Listener&& o) : fd(std::move(o.fd))
			       , plogger(o.plogger)
			       { }
	Listener& operator=(Listener&& o) {
		auto tmp = Listener(std::move(o));
		fd.swap(tmp.fd);
		plogger = tmp.plogger;
		return *this;
	}
	Listener(Listener const&) =delete;

	/* Blocks if not ready.  Use select or poll
	 * on get_fd() to wait for multiple events.
	 */
	Net::SocketFd accept();

	int get_fd() const { return fd.get(); }
};

}

#endif /* CLDCB_COMMON_NET_LISTENER_HPP */

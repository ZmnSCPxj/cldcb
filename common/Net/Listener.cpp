#include<errno.h>
#include<iomanip>
#include<netdb.h>
#include<sstream>
#include<stdexcept>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include"Net/Detail/AddrInfoReleaser.hpp"
#include"Net/Listener.hpp"
#include"Net/SocketFd.hpp"
#include"Util/make_unique.hpp"

namespace {

Net::Fd open_listener(int port) {
	auto portstring = ([port]() {
		auto os = std::ostringstream();
		os << std::dec << port;
		return os.str();
	})();

	auto hint = addrinfo();
	hint.ai_family = AF_UNSPEC;      /* IPv4 or IPv6.  */
	hint.ai_socktype = SOCK_STREAM;  /* TCP.  */
	hint.ai_protocol = 0;
	hint.ai_flags = AI_PASSIVE       /* For listening.  */
		      | AI_NUMERICSERV   /* Service is a port number.  */
		      ;

	auto addrs = Net::Detail::AddrInfoReleaser();
	auto gres = getaddrinfo( nullptr, portstring.c_str()
			       , &hint
			       , &addrs.get()
			       );
	if (gres < 0)
		return Net::Fd(-1);

	for (auto p = addrs.get(); p; p = p->ai_next) {
		auto fd = Net::Fd(socket( p->ai_family
					, p->ai_socktype
					, p->ai_protocol
					));
		if (!fd)
			continue;

		auto bres = bind( fd.get()
				, p->ai_addr, p->ai_addrlen
				);
		if (bres < 0)
			continue;

		auto lres = listen(fd.get(), 128);
		if (lres < 0)
			continue;

		return fd;
	}

	return Net::Fd(-1);
}

}

namespace Net {

Listener::Listener(int port) : fd(open_listener(port)) {
	if (!fd) {
		auto os = std::ostringstream();
		os << "Could not bind to port: " << port;
		throw std::runtime_error(os.str());
	}
}

Net::SocketFd Listener::accept() {
	if (!fd)
		throw std::logic_error("Attempt to use Net::Listener after being moved from.");

	auto ret_fd = Net::Fd();
	do {
		ret_fd = Net::Fd(::accept(fd.get(), nullptr, nullptr));
	} while (!ret_fd && errno == EINTR);

	/* Propagates error as well.  */
	return Net::SocketFd(std::move(ret_fd));
}

}

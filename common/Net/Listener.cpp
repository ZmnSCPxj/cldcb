#include<errno.h>
#include<iomanip>
#include<netdb.h>
#include<sstream>
#include<stdexcept>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include"Net/Detail/AddrInfoReleaser.hpp"
#include"Net/Listener.hpp"
#include"Net/SocketFd.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

std::string describe_sockaddr(sockaddr* sa, socklen_t salen) {
	char hostbuff[NI_MAXHOST];
	char portbuff[NI_MAXSERV];
	memset(hostbuff, 0, NI_MAXHOST);
	memset(portbuff, 0, NI_MAXSERV);
	getnameinfo( sa, salen
		   , hostbuff, sizeof(hostbuff)
		   , portbuff, sizeof(portbuff)
		   , NI_NUMERICHOST | NI_NUMERICSERV
		   );
	return std::string("[")
	     + std::string(hostbuff)
	     + "]:"
	     + std::string(portbuff)
	     ;
}

std::string describe_addrinfo(addrinfo& ai) {
	auto family = (ai.ai_family == AF_INET) ?   "IPv4" :
		      (ai.ai_family == AF_INET6) ?  "IPv6" :
		      /*default*/                   "IP??" ;
	return std::string(family)
	     + " "
	     + describe_sockaddr(ai.ai_addr, ai.ai_addrlen)
	     ;
}

Net::Fd open_listener(int port, Util::Logger& logger) {
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
	if (gres < 0) {
		logger.BROKEN( "Net::Listener(): getaddrinfo: %s"
			     , gai_strerror(gres)
			     );
		if (gres == EAI_SYSTEM)
			logger.BROKEN( "Net::Listener(): getaddrinfo: "
				       "errno: %s"
				     , strerror(errno)
				     );
		return Net::Fd(-1);
	}

	for (auto p = addrs.get(); p; p = p->ai_next) {
		auto addrinfo_str = describe_addrinfo(*p);
		logger.debug( "Attempt to listen on: %s"
			    , addrinfo_str.c_str()
			    );
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

		logger.info( "Listen succeded. <fd %d> on %s."
			   , fd.get()
			   , addrinfo_str.c_str()
			   );

		return fd;
	}

	logger.info("No more addresses to listen on!");
	return Net::Fd(-1);
}

}

namespace Net {

Listener::Listener(int port, Util::Logger& logger_)
		  : fd(open_listener(port, logger_)), plogger(&logger_) {
	auto& logger = *plogger;
	if (!fd) {
		auto os = std::ostringstream();
		os << "Could not bind to port: " << port;
		logger.BROKEN("%s", os.str().c_str());
		throw std::runtime_error(os.str());
	}
}

Net::SocketFd Listener::accept() {
	auto& logger = *plogger;
	if (!fd)
		throw std::logic_error("Attempt to use Net::Listener after being moved from.");

	auto addr = sockaddr_in6();
	auto addrlen = socklen_t(sizeof(addr));

	auto ret_fd = Net::Fd();
	do {
		ret_fd = Net::Fd(::accept( fd.get()
					 , reinterpret_cast<sockaddr*>(&addr)
					 , &addrlen
					 ));
	} while (!ret_fd && errno == EINTR);

	if (ret_fd) {
		logger.info( "Got socket <fd %d> from %s"
			   , ret_fd.get()
			   , describe_sockaddr( reinterpret_cast<sockaddr*>(&addr)
					      , addrlen
					      ).c_str()
			   );
		errno = 0;
	}

	/* Propagates error as well.  */
	return Net::SocketFd(std::move(ret_fd));
}

}

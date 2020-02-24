#ifndef CLDCB_COMMON_NET_CONNECTOR_HPP
#define CLDCB_COMMON_NET_CONNECTOR_HPP

#include<memory>
#include<string>

namespace Net { class SocketFd; }

namespace Net {

/* Interface to an entity that is capable of connecting
 * to a host:port.
 * This is an abstract interface since we might want to
 * use a proxy of some kind.
 */
class Connector {
public:
	virtual ~Connector() { }

	/* Returns null if failed to connect.  */
	virtual
	std::unique_ptr<Net::SocketFd>
	connect(std::string const& host, int port) =0;
};

}

#endif /* CLDCB_COMMON_NET_CONNECTOR_HPP */

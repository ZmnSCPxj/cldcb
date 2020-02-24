#ifndef CLDCB_COMMON_NET_DIRECTCONNECTOR_HPP
#define CLDCB_COMMON_NET_DIRECTCONNECTOR_HPP

#include"Net/Connector.hpp"

namespace Net {

/* A Connector that just connects directly to
 * the specified host:port.
 */
class DirectConnector : public Connector {
public:
	Net::SocketFd
	connect(std::string const& host, int port) override;
};

}

#endif /* CLDCB_COMMON_NET_DIRECTCONNECTOR_HPP */

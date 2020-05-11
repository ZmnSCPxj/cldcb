#ifndef CLDCB_COMMON_NET_UNIXSOCKET_HPP
#define CLDCB_COMMON_NET_UNIXSOCKET_HPP

#include<string>

namespace Net { class SocketFd; }

namespace Net {

Net::SocketFd unixsocket(std::string const& name);

}

#endif /* CLDCB_COMMON_NET_UNIXSOCKET_HPP */

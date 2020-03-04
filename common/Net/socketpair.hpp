#ifndef CLDCB_COMMON_NET_SOCKETPAIR_HPP
#define CLDCB_COMMON_NET_SOCKETPAIR_HPP

#include<utility>

namespace Net { class SocketFd; }

namespace Net {

std::pair<Net::SocketFd, Net::SocketFd> socketpair();

}

#endif /* CLDCB_COMMON_NET_SOCKETPAIR_HPP */

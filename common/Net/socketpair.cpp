#include<errno.h>
#include<stdexcept>
#include<string>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include"Net/SocketFd.hpp"
#include"Net/socketpair.hpp"

namespace Net {

std::pair<Net::SocketFd, Net::SocketFd> socketpair() {
	auto fd0 = Net::SocketFd();
	auto fd1 = Net::SocketFd();

	int fds[2];
	auto res = ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	if (res < 0)
		throw std::runtime_error( std::string("socketpair: ")
					+ strerror(errno)
					);

	fd0.reset(fds[0]);
	fd1.reset(fds[1]);

	return std::make_pair(std::move(fd0), std::move(fd1));
}

}

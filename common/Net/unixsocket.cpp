#include<errno.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include"Net/SocketFd.hpp"
#include"Net/unixsocket.hpp"

namespace Net {

Net::SocketFd unixsocket(std::string const& name) {
	auto fd = Net::Fd(socket(AF_UNIX, SOCK_STREAM, 0));
	if (!fd)
		return Net::SocketFd();

	auto a = sockaddr_un();
	memset(&a, 0, sizeof(a));

	if (name.size() + 1 > sizeof(a.sun_path)) {
		fd.reset();
		errno = ENAMETOOLONG;
		return Net::SocketFd();
	}
	a.sun_family = AF_UNIX;
	strcpy(a.sun_path, name.c_str());

	auto connres = int();
	do {
		connres = connect( fd.get()
				 , reinterpret_cast<sockaddr*>(&a)
				 , sizeof(a)
				 );
	} while (connres < 0 && errno == EINTR);
	if (connres < 0) {
		auto my_errno = errno;
		fd.reset();
		errno = my_errno;
		return Net::SocketFd();
	}

	return Net::SocketFd(std::move(fd));
}

}

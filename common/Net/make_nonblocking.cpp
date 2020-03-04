#include<fcntl.h>
#include<unistd.h>
#include"Net/make_nonblocking.hpp"

namespace Net {

void make_nonblocking(int fd) {
	auto flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

}

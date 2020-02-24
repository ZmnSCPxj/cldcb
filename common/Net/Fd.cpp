#include<errno.h>
#include<unistd.h>
#include<utility>
#include"Net/Fd.hpp"

namespace Net {

Fd& Fd::operator=(Fd&& o) {
	auto tmp = Fd(std::move(o));
	swap(tmp);
	return *this;
}

Fd::~Fd() {
	if (fd >= 0) {
		close(fd);
		/* Ignore errors.  */
		errno = 0;
	}
}

}

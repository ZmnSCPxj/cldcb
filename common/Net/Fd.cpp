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
		/* Ignore errors.
		 * This is a destructor, so it is
		 * entirely possible that this
		 * occurred due to errno-based
		 * errors.
		 * So, we should preserve the errno.
		 */
		auto my_errno = errno;
		close(fd);
		errno = my_errno;
	}
}

}

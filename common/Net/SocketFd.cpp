#include<errno.h>
#include<sys/socket.h>
#include<unistd.h>
#include"Net/SocketFd.hpp"

namespace Net {

SocketFd::~SocketFd() {
	if (!fd)
		return;

	char unused[64];
	auto sys_res = int();

	/* Tell remote side that we will stop sending data and
	 * trigger an EOF there.
	 */
	sys_res = shutdown(fd.get(), SHUT_WR);
	if (sys_res < 0)
		/* Normal ~Fd() will close the socket file descriptor.  */
		return;

	/* Drain the incoming socket until EOF.  */
	for (;;) {
		do {
			sys_res = read(fd.get(), unused, sizeof(unused));
		} while (sys_res < 0 && errno == EINTR);
		/* Let normal ~Fd() close.
		 * sys_res is 0 at EOF, and -1 at error.
		 */
		if (sys_res <= 0)
			return;
	}
}

}


#include<algorithm>
#include<iterator>
#include<stdarg.h>
#include"Ev/Io.hpp"
#include"Ev/wait_io.hpp"
#include"Ev/wait_readable.hpp"

namespace Ev {

Ev::Io<int> wait_readable(std::vector<int> fds) {
	auto fd_dirs = std::vector<std::pair<int, WaitDirection>>();
	std::transform( fds.begin(), fds.end()
		      , std::back_inserter(fd_dirs)
		      , [](int fd) {
		return std::make_pair(fd, Ev::WaitRead);
	});
	/* Free up our input.  */
	fds.clear();
	return wait_io(std::move(fd_dirs));
}
Ev::Io<int> wait_readables(int fd0, ...) {
	auto fds = std::vector<int>();
	fds.push_back(fd0);

	va_list ap;
	va_start(ap, fd0);
	for (;;) {
		auto fd = va_arg(ap, int);
		if (fd < 0)
			break;
		fds.push_back(fd);
	}
	va_end(ap);

	return wait_readable(std::move(fds));
}
Ev::Io<int> wait_readable(int fd) {
	return wait_readables(fd, -1);
}

}

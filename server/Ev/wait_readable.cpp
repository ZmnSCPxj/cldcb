#include<algorithm>
#include<errno.h>
#include<ev.h>
#include<iterator>
#include<memory>
#include<poll.h>
#include<stdarg.h>
#include<utility>
#include"Ev/Io.hpp"
#include"Ev/wait_readable.hpp"
#include"Util/make_unique.hpp"

namespace {

struct WaitReadable {
	std::vector<std::unique_ptr<ev_io>> waiters;
	std::function<void(int)> pass;
};

bool is_read_ready(int fd) {
	auto pollarg = pollfd();
	pollarg.fd = fd;
	pollarg.events = POLLIN;
	pollarg.revents = 0;
	auto res = int();
	do {
		res = poll(&pollarg, 1, 0);
	} while (res < 0 && errno == EINTR);
	if (res < 0)
		/* Cannot determine... */
		return true;
	return (pollarg.revents & POLLIN) != 0;
}

void read_waiter_handler(EV_P_ ev_io *raw_read_waiter, int events) {
	/* Do a single poll to check if the fd is really okay.  */
	if (is_read_ready(raw_read_waiter->fd)) {
		/* Cache the fd that succeeded.  */
		auto fd = raw_read_waiter->fd;

		/* Acquire ownership of the WaitReadable.  */
		auto wait_readable = std::unique_ptr<WaitReadable>();
		wait_readable.reset((WaitReadable*)raw_read_waiter->data);
		/* Cache the pass function.  */
		auto pass = std::move(wait_readable->pass);

		/* Stop all waiters.
		 * This lets us safely release the data structure.
		 */
		for (auto& w : wait_readable->waiters)
			ev_io_stop(EV_DEFAULT_ w.get());

		/* Release the data structure.  */
		wait_readable = nullptr;

		/* Pass the successful fd.  */
		pass(fd);
	} else {
		/* Re-arm.  */
		ev_io_start(EV_DEFAULT_ raw_read_waiter);
	}
}

class IoWaitReadable {
private:
	std::vector<int> fds;

public:
	IoWaitReadable(IoWaitReadable&&) =default;
	IoWaitReadable(IoWaitReadable const&) =default;
	/* The only purpose of this class (instead of using a lambda)
	 * is to be able to move-construct the fds vector.
	 */
	IoWaitReadable(std::vector<int> fds_)
		: fds(std::move(fds_)) { }

	void operator()( std::function<void(int)> pass
		       , std::function<void(std::exception)> fail
		       ) {
		auto wait_readable = Util::make_unique<WaitReadable>();
		/* Cache the pass function.  */
		wait_readable->pass = std::move(pass);
		/* Create the waiters.  */
		std::transform( fds.begin(), fds.end()
			      , std::back_inserter(wait_readable->waiters)
			      , [&wait_readable] (int fd) {
			auto ret = Util::make_unique<ev_io>();
			ev_io_init( ret.get(), &read_waiter_handler
				  , fd, EV_READ
				  );
			ret->data = wait_readable.get();

			return ret;
		});
		/* Now that all waiters were successfully created,
		 * arm them.
		 * This is a separate loop from the above, because
		 * in case the above loop throws (e.g. out of memory),
		 * the waiters can be freed by normal destruction of
		 * objects, and the waiters do not have to be disarmed.
		 * The loop below is guaranteed not to throw, since it
		 * only calls C code and performs iterator operations.
		 */
		for (auto& w : wait_readable->waiters)
			ev_io_start(EV_DEFAULT_ w.get());
		/* The armed waiters are now the responsibility of
		 * libev.
		 * The armed waiters keep the WaitReadable structure
		 * alive; as soon as one of the waiters succeeds,
		 * that one takes responsibility for the WaitReadable
		 * and frees everyone.
		 */
		(void) wait_readable.release();
	}
};

}

namespace Ev {

Ev::Io<int> wait_readable(std::vector<int> fds) {
	return Io<int>(IoWaitReadable(std::move(fds)));
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

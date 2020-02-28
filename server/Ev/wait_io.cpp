#include<algorithm>
#include<errno.h>
#include<ev.h>
#include<iterator>
#include<poll.h>
#include<stdarg.h>
#include"Ev/Io.hpp"
#include"Ev/wait_io.hpp"
#include"Util/make_unique.hpp"

namespace {

struct WaitIo {
	std::vector<std::unique_ptr<ev_io>> waiters;
	std::function<void(int)> pass;
};

bool is_ready(int fd, int revent) {
	auto pollarg = pollfd();
	pollarg.fd = fd;
	auto events = pollarg.events = (revent & EV_READ) ? POLLIN : POLLOUT;
	pollarg.revents = 0;

	auto res = int();
	do {
		res = poll(&pollarg, 1, 0);
	} while (res < 0 && errno == EINTR);
	if (res < 0)
		/* Cannot determine... */
		return true;
	return (pollarg.revents & events) != 0;
}

void io_waiter(EV_P_ ev_io *raw_io_waiter, int revents) {
	/* Check if the fd is really okay.  */
	if (is_ready(raw_io_waiter->fd, revents)) {
		/* Cache the fd that succeeded.  */
		auto fd = raw_io_waiter->fd;

		/* Acquire ownership of the WaitIo.  */
		auto wait_io = std::unique_ptr<WaitIo>();
		wait_io.reset((WaitIo*)raw_io_waiter->data);
		/* Cache the pass function.  */
		auto pass = std::move(wait_io->pass);

		/* Stop all waiters.
		 * This lets us safely release the data structure.
		 */
		for (auto& w : wait_io->waiters)
			ev_io_stop(EV_A_ w.get());

		/* Release the data structure; we have cached what
		 * we need.
		 */
		wait_io = nullptr;

		/* Pass the succeeding fd.  */
		pass(fd);
	} else {
		/* Re-arm.  */
		ev_io_start(EV_A_ raw_io_waiter);
	}
}

/* A functor that we can pass to Io<int>().  */
class WaitIoFunctor {
private:
	std::vector<std::pair<int, Ev::WaitDirection>> fds;

public:
	WaitIoFunctor(WaitIoFunctor&&) =default;
	WaitIoFunctor(WaitIoFunctor const&) =default;
	/* The only purpose of this class (instead of using a lambda)
	 * is to be able to move-construct the fds vector.
	 */
	WaitIoFunctor(std::vector<std::pair<int, Ev::WaitDirection>> fds_)
		: fds(std::move(fds_)) { }

	void operator()( std::function<void(int)> pass
		       , std::function<void(std::exception)> fail
		       ) {
		auto wait_io = Util::make_unique<WaitIo>();
		/* Save the pass function.  */
		wait_io->pass = std::move(pass);
		/* Create the waiters.  */
		std::transform( fds.begin(), fds.end()
			      , std::back_inserter(wait_io->waiters)
			      , [&wait_io] (std::pair<int, Ev::WaitDirection> const& fd_dir) {
			auto fd = fd_dir.first;
			auto dir = fd_dir.second;
			auto ret = Util::make_unique<ev_io>();
			ev_io_init( ret.get(), &io_waiter
				  , fd
				  , (dir == Ev::WaitRead) ? EV_READ : EV_WRITE
				  );
			ret->data = wait_io.get();

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
		for (auto& w : wait_io->waiters)
			ev_io_start(EV_DEFAULT_ w.get());
		/* The armed waiters are now the responsibility of
		 * libev.
		 * The armed waiters keep the WaitIo structure alive;
		 * as soon as one of the waiters succeeds, that one
		 * takes responsibility for the WaitIo and frees
		 * everyone.
		 */
		(void) wait_io.release();
	}
};

}

namespace Ev {

Ev::Io<int> wait_io(std::vector<std::pair<int, WaitDirection>> fds) {
	return Io<int>(WaitIoFunctor(std::move(fds)));
}

Ev::Io<int> wait_io(int fd0, WaitDirection dir0, ...) {
	auto fds = std::vector<std::pair<int, WaitDirection>>();
	fds.push_back(std::make_pair(fd0, dir0));

	va_list ap;
	va_start(ap, dir0);
	for (;;) {
		auto fd = va_arg(ap, int);
		if (fd < 0)
			break;
		auto dir = (WaitDirection)(va_arg(ap, int));
		fds.push_back(std::make_pair(fd, dir));
	}
	va_end(ap);

	return wait_io(std::move(fds));
}

}

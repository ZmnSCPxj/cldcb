#include<assert.h>
#include<errno.h>
#include<poll.h>
#include<stdexcept>
#include<string.h>
#include<unistd.h>
#include"ServerTalker/Detail/QueueTalker.hpp"

namespace ServerTalker { namespace Detail {

QueueTalker::QueueTalker(int fd_) : fd(fd_)
				  , aborted(false)
				  {
	int pipes[2];
	auto piperes = pipe(pipes);
	if (piperes < 0) {
		auto my_errno = errno;
		throw std::runtime_error
			( std::string("Failed to create pipe: ")
			+ strerror(my_errno)
			);
	}

	pipe_in = Net::Fd(pipes[1]);
	pipe_out = Net::Fd(pipes[0]);
}

QueueTalker::Result QueueTalker::wait(QueueTalker::WaitType wait_for) {
	pollfd to_poll[2];

	to_poll[0].fd = pipe_out.get();
	to_poll[0].events = POLLIN;
	to_poll[0].revents = 0;
	to_poll[1].fd = (wait_for == Neither) ? -1 : fd;
	switch (wait_for) {
	case Read: to_poll[1].events = POLLIN; break;
	case Write: to_poll[1].events = POLLOUT; break;
	case ReadWrite: to_poll[1].events = POLLIN | POLLOUT; break;
	case Neither: to_poll[1].events = 0; break;
	}
	to_poll[1].revents = 0;

	auto pollres = int();
	do {
		pollres = poll(to_poll, 2, -1);
	} while (pollres < 0 && errno == EINTR);
	if (pollres < 0) {
		auto my_errno = errno;
		throw std::runtime_error
			( std::string("Failed to poll: ")
			+ strerror(my_errno)
			);
	}

	auto ret = Result();

	if (to_poll[0].revents != 0) {
		/* Something was signalled on the queue, check what it was.  */
		auto lock = std::unique_lock<std::mutex>(mtx);
		if (aborted) {
			ret.type = Abort;
			return ret;
		}

		/* Must be a new item.  Pull from the queue.  */
		assert(!items.empty());
		ret.type = ItemAvailable;
		ret.item = std::move(items.front());
		items.pop();

		/* Got everything from mutex-protected section;
		 * release mutex.
		 */
		lock.unlock();

		/* Now clear the output end of the pipe.  */
		auto c = char();
		auto readres = ssize_t();
		do {
			readres = read(pipe_out.get(), &c, 1);
		} while (readres < 0 && errno == EINTR);
		if (readres < 0) {
			auto my_errno = errno;
			throw std::runtime_error
				( std::string("Failed to read: ")
				+ strerror(my_errno)
				);
		}

		return ret;
	} else {
		/* Must be other fd that is awoken.  */
		ret.type = FdAvailable;
		return ret;
	}
}

void QueueTalker::post(std::unique_ptr<QueueTalker::Item> item) {
	auto lock = std::unique_lock<std::mutex>(mtx);
	items.emplace(std::move(item));
	lock.unlock();
	raise_pipe();
}

void QueueTalker::abort() {
	auto lock = std::unique_lock<std::mutex>(mtx);
	aborted = true;
	lock.unlock();
	raise_pipe();
}

void QueueTalker::raise_pipe() {
	/* Now post to the pipe.  */
	auto c = char(0);
	auto writeres = ssize_t();
	do {
		writeres = write(pipe_in.get(), &c, 1);
	} while (writeres < 0 && errno == EINTR);
	if (writeres < 0) {
		auto my_errno = errno;
		throw std::runtime_error
			( std::string("Failed to write: ")
			+ strerror(my_errno)
			);
	}
}

}}

#include<errno.h>
#include<fcntl.h>
#include<poll.h>
#include<signal.h>
#include<stdexcept>
#include<string.h>
#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Daemon/IoResult.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/wait_io.hpp"
#include"Util/Logger.hpp"

namespace {

auto breaker_initialized = false;

/* The write end of the signalling pipe.
 * When a SIGTERM or SIGINT is received, the signal handler
 * just writes to the signalling pipe.
 * The application (Daemon::Breaker object) monitors the
 * read end of the pipe; if it becomes readable, then that
 * is our indication that one of those signals has been
 * received.
 */
auto pipe_write = int(-1);

void break_sig_handler(int) {
	auto const buffer = char(1);
	if (pipe_write >= 0) {
		auto res = write(pipe_write, &buffer, 1);
		/* Not that we can do anything, but shuts up
		 * compiler warnings.
		 */
		if (res < 0)
			return;
	}
}
void make_nonblocking(int fd) {
	auto flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
}

}

namespace Daemon {

Breaker::Breaker(Util::Logger& logger_) : logger(logger_) {
	struct sigaction sa;
	sa.sa_handler = &break_sig_handler;
	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	auto res_int = sigaction(SIGINT, &sa, nullptr);
	if (res_int < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Daemon::Breaker: sigaction(SIGINT): %s"
			     , strerror(my_errno)
			     );
		throw std::runtime_error(std::string("Daemon::Breaker: sigaction(SIGINT):") + strerror(my_errno));
	}
	auto res_term = sigaction(SIGTERM, &sa, nullptr);
	if (res_term < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Daemon::Breaker: sigaction(SIGTERM): %s"
			     , strerror(my_errno)
			     );
		signal(SIGINT, SIG_DFL);
		throw std::runtime_error(std::string("Daemon::Breaker: sigaction(SIGTERM):") + strerror(my_errno));
	}

	auto res_sigpipe = signal(SIGPIPE, SIG_IGN);
	if (res_sigpipe < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Daemon::Breaker: signal(SIGPIPE): %s"
			     , strerror(my_errno)
			     );
		signal(SIGINT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		throw std::runtime_error(std::string("Daemon::Breaker: signal(SIGPIPE):") + strerror(my_errno));
	}

	int pipes[2];
	auto res_pipe = pipe(pipes);
	if (res_pipe < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Daemon::Breaker: pipe: %s"
			     , strerror(my_errno)
			     );
		signal(SIGPIPE, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		throw std::runtime_error(std::string("Daemon::Breaker: pipe:") + strerror(my_errno));
	}

	pipe_read = pipes[0];
	pipe_write = pipes[1];

	make_nonblocking(pipe_read);
	make_nonblocking(pipe_write);
}
Breaker::~Breaker() {
	/* Ignore errors; we are cleaning up!  */
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	close(pipe_write);
	close(pipe_read);
	signal(SIGPIPE, SIG_DFL);
}

std::unique_ptr<Breaker> Breaker::initialize(Util::Logger& logger) {
	if (breaker_initialized)
		throw std::logic_error("Already initialized breaker!");
	/* Cannot use make_unique; Breaker constructor is private.  */
	auto ret = std::unique_ptr<Breaker>(new Breaker(logger));

	breaker_initialized = true;

	return ret;
}

Ev::Io<bool> Breaker::wait_readable_or_break(int fd) {
	return Ev::wait_io( fd, Ev::WaitRead
			  , pipe_read, Ev::WaitRead
			  , -1
			  ).then<bool>([this](int ready_fd) {
		if (ready_fd == pipe_read) {
			logger.debug("Detected break signal while waiting for fd to be readable.");
			return Ev::lift_io(false);
		} else
			return Ev::lift_io(true);
	});
}
Ev::Io<bool> Breaker::wait_writeable_or_break(int fd) {
	return Ev::wait_io( fd, Ev::WaitWrite
			  , pipe_read, Ev::WaitRead
			  , -1
			  ).then<bool>([this](int ready_fd) {
		if (ready_fd == pipe_read) {
			logger.debug("Detected break signal while waiting for fd to be writeable.");
			return Ev::lift_io(false);
		} else
			return Ev::lift_io(true);
	});
}
Ev::Io<bool> Breaker::is_unbroken() {
	return Ev::Io<bool>([this]( std::function<void(bool)> pass
				  , std::function<void(std::exception)> fail
				  ) {
		auto pollarg = pollfd();
		pollarg.fd = pipe_read;
		pollarg.events = POLLIN;
		pollarg.revents = 0;

		auto res = int();
		do {
			res = poll(&pollarg, 1, 0);
		} while (res < 0 && errno == EINTR);
		if (res < 0)
			pass(false);
		else
			pass((pollarg.revents & POLLIN) != 0);
	});
}

namespace {

class ReadTimed {
private:
	/* Shaed-pointer-to-this pattern.
	 * We want to use a shared pointer to this since we need
	 * to keep this object alive while we are waiting.
	 */
	std::weak_ptr<ReadTimed> weak_self;

	std::shared_ptr<ReadTimed> get_self() const {
		return std::shared_ptr<ReadTimed>(weak_self);
	}

	int fd;
	std::size_t bytes;
	bool has_timeout;
	double endtime;
	std::vector<std::uint8_t> data;
	int break_fd;
	Util::Logger& logger;

	ReadTimed( int fd_
		 , std::size_t bytes_
		 , double timeout_
		 , std::vector<std::uint8_t> data_
		 , int break_fd_
		 , Util::Logger& logger_
		 ) : fd(fd_)
		   , bytes(bytes_)
		   , has_timeout(timeout_ >= 0.0)
		   , endtime(Ev::now() + timeout_)
		   , data(std::move(data))
		   , break_fd(break_fd_)
		   , logger(logger_)
		   { }

public:
	static
	std::shared_ptr<ReadTimed>
	create( int fd
	      , std::size_t bytes
	      , double timeout
	      , std::vector<std::uint8_t> data
	      , int break_fd
	      , Util::Logger& logger
	      ) {
		auto ret = std::shared_ptr<ReadTimed>();
		ret.reset(new ReadTimed( fd
				       , bytes
				       , timeout
				       , std::move(data)
				       , break_fd
				       , logger
				       ));
		ret->weak_self = ret;
		return ret;
	}

public:
	Ev::Io<IoResult>
	perform() {
		if (data.size() >= bytes)
			return Ev::lift_io(IoResult{IoOk, std::move(data)});
		else if (has_timeout && Ev::now() >= endtime)
			return Ev::lift_io(IoResult{ IoTimeout
						   , std::move(data)
						   });
		auto self = get_self();
		auto timediff = has_timeout ? endtime - Ev::now() : -1.0;
		return Ev::wait_io_until( fd, Ev::WaitRead
					, break_fd, Ev::WaitRead
					, -1, timediff
					).then<IoResult>([self](int fd) {
			return self->postwait(fd);
		});
	}

private:
	Ev::Io<IoResult>
	postwait(int fd) {
		if (fd == break_fd)
			return Ev::lift_io(IoResult{ IoBroken
						   , std::move(data)
						   });
		else if (fd < 0)
			return Ev::lift_io(IoResult{ IoTimeout
						   , std::move(data)
						   });
		auto offset = data.size();
		auto to_read = bytes - data.size();
		/* Temporarily resize data.  */
		data.resize(bytes);

		/* Make a single attempt at reading.  */
		auto res = read(fd, &data[offset], to_read);
		if (res < 0) {
			auto my_errno = errno;
			/* Reset resizing.  */
			data.resize(offset);

			/* For these errors, retry.
			 * In particular, for EINTR, the interrupt
			 * might have been a SIGINT/SIGTERM, so we
			 * should loop back to also check the break_fd.
			 */
			if ( my_errno == EINTR
			  || my_errno == EAGAIN
			  || my_errno == EWOULDBLOCK
			   )
				return perform();

			/* Report, then abort.... */
			logger.BROKEN("While reading from <fd %d>: %s"
				     , fd
				     , strerror(my_errno)
				     );
			return Ev::lift_io(IoResult{ IoBroken
						   , std::move(data)
						   });
		}

		/* Resize back to however much was read.  */
		data.resize(offset + std::size_t(res));
		/* If size was 0, that was an EOF.  */
		if (res == 0)
			return Ev::lift_io(IoResult{ IoEof
						   , std::move(data)
						   });
		return perform();
	}
};

}

Ev::Io<IoResult>
Breaker::read_timed( int fd
		   , std::size_t bytes
		   , double timeout
		   , std::vector<std::uint8_t> data
		   ) {
	data.reserve(bytes);
	auto handler = ReadTimed::create( fd
					, bytes
					, timeout
					, std::move(data)
					, pipe_read
					, logger
					);
	return handler->perform();
}

namespace {

class WriteTimed {
private:
	std::weak_ptr<WriteTimed> weak_self;

	std::shared_ptr<WriteTimed> get_self() const {
		return std::shared_ptr<WriteTimed>(weak_self);
	}

	int fd;
	std::vector<std::uint8_t> data;
	std::vector<std::uint8_t>::const_iterator start;
	bool has_timeout;
	double endtime;
	int break_fd;
	Util::Logger& logger;

	WriteTimed( int fd_
		  , std::vector<std::uint8_t> data_
		  , double timeout_
		  , int break_fd_
		  , Util::Logger& logger_
		  ) : fd(fd_)
		    , data(std::move(data_))
		    , start(data.cbegin())
		    , has_timeout(timeout_ >= 0.0)
		    , endtime(Ev::now() + timeout_)
		    , break_fd(break_fd_)
		    , logger(logger_)
		    { }

public:
	static
	std::shared_ptr<WriteTimed>
	create( int fd
	      , std::vector<std::uint8_t> data
	      , double timeout
	      , int break_fd
	      , Util::Logger& logger
	      ) {
		auto ret = std::shared_ptr<WriteTimed>();
		ret.reset(new WriteTimed( fd
					, std::move(data)
					, timeout
					, break_fd
					, logger
					));
		ret->weak_self = ret;
		return ret;
	}

private:
	Ev::Io<IoResult>
	return_result(IoResultType type) {
		/* Remove the data that was already written.  */
		data.erase(data.begin(), start);

		return Ev::lift_io(IoResult{type, std::move(data)});
	}

public:
	Ev::Io<IoResult>
	perform() {
		if (start == data.end())
			/* Nothing to write!  */
			return return_result(IoOk);
		else if (has_timeout && Ev::now() >= endtime)
			/* Timed out while writing.  */
			return return_result(IoTimeout);

		auto self = get_self();
		auto timeout = has_timeout ? endtime - Ev::now() : -1.0;
		return Ev::wait_io_until( fd, Ev::WaitWrite
					, break_fd, Ev::WaitRead
					, -1, timeout
					).then<IoResult>([self](int fd) {
			return self->postwait(fd);
		});
	}

private:
	Ev::Io<IoResult>
	postwait(int fd) {
		if (fd == break_fd)
			return return_result(IoBroken);
		else if (fd < 0)
			return return_result(IoTimeout);

		/* Make a single write attempt.  */
		auto to_write = data.cend() - start;
		auto res = write(fd, &*start, to_write);
		if (res < 0) {
			/* If interrupted or would block, just loop back.  */
			if ( errno == EINTR
			  || errno == EAGAIN
			  || errno == EWOULDBLOCK
			   )
				return perform();
			/* If EPIPE, report EOF.  */
			if (errno == EPIPE)
				return return_result(IoEof);

			/* Otherwise log the error and report broken.  */
			logger.BROKEN( "While writing to <fd %d>: %s"
				     , fd, strerror(errno)
				     );
			return return_result(IoBroken);
		}

		/* Advance the start-pointer.  */
		start += std::size_t(res);
		/* Loop.  */
		return perform();
	}

};

}

Ev::Io<IoResult>
Breaker::write_timed( int fd
		    , std::vector<std::uint8_t> data
		    , double timeout
		    ) {
	auto handler = WriteTimed::create( fd
					 , std::move(data)
					 , timeout
					 , pipe_read
					 , logger
					 );
	return handler->perform();
}

}

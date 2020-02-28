#include<fcntl.h>
#include<poll.h>
#include<signal.h>
#include<stdexcept>
#include<string.h>
#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Ev/Io.hpp"
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

}

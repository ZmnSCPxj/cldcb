#ifndef CLDCB_SERVER_DAEMON_BREAKER_HPP
#define CLDCB_SERVER_DAEMON_BREAKER_HPP

#include<memory>

namespace Ev { template<typename a> class Io; }
namespace Util { class Logger; }

namespace Daemon {

/* This is a singleton because POSIX signals are singletons.
 *
 * This captures these signals:
 * * SIGTERM
 * * SIGINT
 *
 * If any of the signals are received, then the daemon is
 * "broken" and every thread waiting on network I/O should
 * stop processing.
 *
 * This also sets up SIGPIPE to be ignored, which is
 * necessary in general.
 */
class Breaker {
private:
	Util::Logger& logger;
	int pipe_read;

	/* Only accessible via singleton access!  */
	Breaker(Util::Logger& logger);

public:
	Breaker(Breaker&&) =delete;
	Breaker(Breaker const&) =delete;

	~Breaker();

	/* Initialize exactly once.  Multiple calls will throw.  */
	static
	std::unique_ptr<Breaker> initialize(Util::Logger& logger);

	/* Block until the specified fd is readable, or
	 * until a breaking signal has been received.
	 * If a breaking signal has been received, this
	 * will return immediately.
	 * Return true if the fd is readable, false if
	 * the breaking signal has been received.
	 */
	Ev::Io<bool> wait_readable_or_break(int fd);
	/* Block until the specified fd is writeable,
	 * or until a breaking signal has been
	 * received.
	 * Return true if the fd is writeable, false
	 * if the breaking signal has been received.
	 */
	Ev::Io<bool> wait_writeable_or_break(int fd);

	/* Determine if we are already broken.
	 * Return true if we can continue processing,
	 * false if the breaking signal has been
	 * received.
	 */
	Ev::Io<bool> is_unbroken();
};

}

#endif /* CLDCB_SERVER_DAEMON_BREAKER_HPP */

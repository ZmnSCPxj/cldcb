#ifndef CLDCB_SERVER_DAEMON_PIDFILER_HPP
#define CLDCB_SERVER_DAEMON_PIDFILER_HPP

#include<string>

namespace Util { class Logger; }

namespace Daemon {

/* This class, when constructed, attempts to create
 * the specified pid file and stores the pid into
 * it.
 * On destruction, it attempts to unlink the pid
 * file.
 */
class PidFiler {
private:
	Util::Logger& logger;
	std::string pid_path;

public:
	PidFiler() =delete;
	PidFiler(PidFiler const&) =delete;
	PidFiler(PidFiler&&) =delete;

	explicit PidFiler( Util::Logger& logger
			 , std::string pid_path
			 );
	~PidFiler();
};

}

#endif /* CLDCB_SERVER_DAEMON_PIDFILER_HPP */

#ifndef CLDCB_SERVER_SERVER_TERMLOGGER_HPP
#define CLDCB_SERVER_SERVER_TERMLOGGER_HPP

#include"Util/Logger.hpp"

namespace Server {

class TermLogger : public Util::Logger {
private:
	bool show_debug;
public:
	explicit TermLogger(bool show_debug_ = false)
		: show_debug(show_debug_) { }
	void log(Util::Logger::LogLevel level, std::string msg) override;
};

}

#endif /* CLDCB_SERVER_SERVER_TERMLOGGER_HPP */

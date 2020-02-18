#ifndef CLDCB_COMMON_UTIL_LOGGER_HPP
#define CLDCB_COMMON_UTIL_LOGGER_HPP

#include<string>

namespace Util {

class Logger {
public:
	virtual ~Logger() { }

	enum LogLevel {
		Debug, Info, Unusual, Broken
	};

	virtual void log(LogLevel, std::string) =0;

	void debug(char const *fmt, ...);
	void info(char const *fmt, ...);
	void unusual(char const *fmt, ...);
	void BROKEN(char const *fmt, ...);
};

}

#endif /* CLDCB_COMMON_UTIL_LOGGER_HPP */

#ifndef CLDCB_SERVER_SERVER_LOGGER_HPP
#define CLDCB_SERVER_SERVER_LOGGER_HPP

#include<memory>
#include<string>
#include<utility>
#include"Util/Logger.hpp"

namespace Server {

class Logger : public Util::Logger {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Logger() =delete;
	Logger(Logger&&) =default;
	Logger& operator=(Logger&&) =default;
	~Logger();
	explicit Logger( std::string const& log_path
		       , Util::Logger::LogLevel min_log_level
				= Util::Logger::Debug
		       );

	void log(Util::Logger::LogLevel, std::string) override;
};

}

#endif /* CLDCB_SERVER_SERVER_LOGGER_HPP */


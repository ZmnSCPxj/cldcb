#include<chrono>
#include<fstream>
#include<stdexcept>
#include<time.h>
#include"Server/Logger.hpp"
#include"Sync/MVar.hpp"
#include"Util/make_unique.hpp"

namespace Server {

class Logger::Impl {
private:
	Sync::MVar<std::ofstream> mlog;
	Util::Logger::LogLevel min_log_level;

public:
	Impl() =delete;
	Impl( std::string const& log_path
	    , Util::Logger::LogLevel min_log_level_
	    ) : mlog(Sync::MVar<std::ofstream>(
			std::ofstream(log_path, std::ios_base::out | std::ios_base::app)
		))
	      , min_log_level(min_log_level_)
	      {
		auto logstream = mlog.take();
		try {
			logstream << std::endl << std::endl;
		} catch (...) { }
		mlog.put(std::move(logstream));
	}

	void log( Util::Logger::LogLevel log_level
		, std::string message
		) {
		if (min_log_level == Util::Logger::Debug)
			/* Nothing.  */ ;
		else if (min_log_level == Util::Logger::Info) {
			if (log_level == Util::Logger::Debug)
				return;
		} else if (min_log_level == Util::Logger::Unusual) {
			if (log_level == Util::Logger::Debug)
				return;
			if (log_level == Util::Logger::Info)
				return;
		} else if (min_log_level == Util::Logger::Broken) {
			if (log_level != Util::Logger::Broken)
				return;
		}

		/* Get current time.  */
		auto now = std::chrono::system_clock::now();
		auto now_tt = std::chrono::system_clock::to_time_t(now);
		auto now_str = std::string(ctime(&now_tt));
		/* Remove trailing \n.  */
		now_str.erase(now_str.end() - 1);

		/* Get level.  */
		auto level = std::string();
		switch (log_level) {
		case Util::Logger::Debug: level = "(debug)"; break;
		case Util::Logger::Info: level = "(info)"; break;
		case Util::Logger::Unusual: level = "(UNUSUAL)"; break;
		case Util::Logger::Broken: level = "***BROKEN***"; break;
		}

		auto prefix = now_str + ": " + level + " ";

		/* TODO: Split message by lines,
		 * add the prefix to each line.  */

		message = prefix + message + "\n";

		/* Assures atomic access even from multiple threads.  */
		auto logstream = mlog.take();
		/* Ignore exceptions in writing to the log.  */
		try {
			logstream << message;
			logstream.flush();
		} catch (...) { }
		mlog.put(std::move(logstream));
	}
};

Logger::~Logger() { }
Logger::Logger( std::string const& log_path
	      , Util::Logger::LogLevel min_log_level
	      ) : pimpl(Util::make_unique<Impl>(log_path, min_log_level))
		{ }

void Logger::log(Util::Logger::LogLevel log_level, std::string message) {
	if (!pimpl)
		throw std::logic_error("Server::Logger used after being moved from.");
	pimpl->log(log_level, std::move(message));
}

}


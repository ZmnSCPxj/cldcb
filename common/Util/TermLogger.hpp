#ifndef CLDCB_COMMON_UTIL_TERMLOGGER_HPP
#define CLDCB_COMMON_UTIL_TERMLOGGER_HPP

#include<iostream>
#include"Util/Logger.hpp"

namespace Util {

class TermLogger : public Util::Logger {
private:
	bool show_debug;
	std::ostream& cout;
	std::ostream& cerr;
public:
	explicit TermLogger( bool show_debug_ = false
			   , std::ostream& cout_ = std::cout
			   , std::ostream& cerr_ = std::cerr
			   ) : show_debug(show_debug_)
			     , cout(cout_)
			     , cerr(cerr_)
			     { }
	void log(Util::Logger::LogLevel level, std::string msg) override;
};

}

#endif /* CLDCB_COMMON_UTIL_TERMLOGGER_HPP */

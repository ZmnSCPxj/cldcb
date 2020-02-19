#include<assert.h>
#include<sstream>
#include"Jsmn/jsonify_string.hpp"
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"

namespace {

char const *log_level_string(Util::Logger::LogLevel l) {
	switch (l) {
	case Util::Logger::Debug: return "debug";
	case Util::Logger::Info: return "info";
	case Util::Logger::Unusual: return "warn";
	case Util::Logger::Broken: return "error";
	}
	assert(0 == 1);
}

}

namespace LD {

void Logger::log( Util::Logger::LogLevel l
		, std::string m
		) {
	std::ostringstream os;
	os << "{ \"jsonrpc\": \"2.0\""
	   << ", \"method\": \"log\""
	   << ", \"params\": { \"level\": \"" << log_level_string(l) << "\""
	   << "              , \"message\": " << Jsmn::jsonify_string(m)
	   << "              }"
	   << "}"
	    ;
	os.flush();
	writer.write(os.str());
}

}

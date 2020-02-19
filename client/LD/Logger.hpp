#ifndef CLDCB_CLIENT_LD_LOGGER_HPP
#define CLDCB_CLIENT_LD_LOGGER_HPP

#include"Util/Logger.hpp"

namespace LD { class Writer; }

namespace LD {

/* A logger which outputs JSONRPC 2.0 "log" notifications
 * to the given writer.
 *
 * { "jsonrpc" : "2.0", "method" : "log"
 * , "params" : { "level" : "info"
 *              , "message" : "Informing you of something..."
 *              }
 * }
 */
class Logger : public Util::Logger {
private:
	LD::Writer& writer;

public:
	Logger() =delete;
	explicit Logger(LD::Writer& writer_) : writer(writer_) { }

	void log(Util::Logger::LogLevel, std::string) override;
};

}

#endif /* CLDCB_CLIENT_LD_LOGGER_HPP */

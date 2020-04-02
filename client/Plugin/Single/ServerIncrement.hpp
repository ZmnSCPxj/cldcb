#ifndef CLDCB_CLIENT_PLUGIN_SINGLE_SERVERINCREMENT_HPP
#define CLDCB_CLIENT_PLUGIN_SINGLE_SERVERINCREMENT_HPP

#include"Plugin/ServerIncrementIf.hpp"

namespace ServerTalker { namespace Detail { class DePinger; }}
namespace Util { class Logger; }

namespace Plugin { namespace Single {

class ServerIncrement : public Plugin::ServerIncrementIf {
private:
	Util::Logger& logger;
	ServerTalker::Detail::DePinger& depinger;

	void init();

public:
	ServerIncrement( Util::Logger& logger_
		       , ServerTalker::Detail::DePinger& depinger_
		       ) : logger(logger_)
			 , depinger(depinger_)
			 {
		init();
	}

	std::future<bool>
	send_increment_chunk(std::vector<std::uint8_t> ciphertext) override;
	std::future<Plugin::ServerResult>
	increment_completed() override;
};

}}

#endif /* CLDCB_CLIENT_PLUGIN_SINGLE_SERVERINCREMENT_HPP */

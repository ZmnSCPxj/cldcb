#ifndef CLDCB_CLIENT_PLUGIN_SINGLE_SERVERINCREMENT_HPP
#define CLDCB_CLIENT_PLUGIN_SINGLE_SERVERINCREMENT_HPP

#include"Plugin/ServerIncrementIf.hpp"

namespace ServerTalker { class Thread; }
namespace Util { class Logger; }

namespace Plugin { namespace Single {

class ServerIncrement : public Plugin::ServerIncrementIf {
private:
	Util::Logger& logger;
	ServerTalker::Thread& server_thread;

	void init();

public:
	ServerIncrement( Util::Logger& logger_
		       , ServerTalker::Thread& server_thread_
		       ) : logger(logger_)
			 , server_thread(server_thread_)
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

#ifndef CLDCB_CLIENT_PLUGIN_SINGLE_SERVERREUPLOAD_HPP
#define CLDCB_CLIENT_PLUGIN_SINGLE_SERVERREUPLOAD_HPP

#include"Plugin/ServerReuploadIf.hpp"

namespace ServerTalker { class Thread; }
namespace Util { class Logger; }

namespace Plugin { namespace Single {

class ServerReupload : public Plugin::ServerReuploadIf {
private:
	Util::Logger& logger;
	ServerTalker::Thread& server_thread;

	void init();

public:
	ServerReupload( Util::Logger& logger_
		      , ServerTalker::Thread& server_thread_
		      ) : logger(logger_)
			, server_thread(server_thread_)
			{
		init();
	}

	std::future<bool>
	send_reupload_chunk(std::vector<std::uint8_t> ciphertext) override;
	std::future<bool>
	reupload_completed() override;
};

}}

#endif /* CLDCB_CLIENT_PLUGIN_SINGLE_SERVERREUPLOAD_HPP */

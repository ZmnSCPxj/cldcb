#ifndef CLDCB_CLIENT_PLUGIN_SINGLE_SERVER_HPP
#define CLDCB_CLIENT_PLUGIN_SINGLE_SERVER_HPP

#include"Plugin/ServerIf.hpp"
#include"ServerTalker/Thread.hpp"

namespace Util { class Logger; }

namespace Plugin { namespace Single {

class Server : public Plugin::ServerIf {
private:
	Util::Logger& logger;
	ServerTalker::Thread server_thread;

public:
	Server() =delete;
	Server(Server&&) =delete;

	explicit
	Server( Util::Logger& logger_
	      , std::function<std::unique_ptr<ServerTalker::MessengerIf>()>
			messenger_constructor
	      ) : logger(logger_)
		, server_thread(logger_, messenger_constructor)
		{ }

	std::future<std::unique_ptr<Plugin::ServerIncrementIf>>
	new_update( std::uint32_t data_version
		  ) override;
};

}}

#endif /* CLDCB_CLIENT_PLUGIN_SINGLE_SERVER_HPP */

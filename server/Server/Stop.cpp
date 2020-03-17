#include<iostream>
#include<signal.h>
#include"Server/Stop.hpp"
#include"Server/OptionsHandler.hpp"
#include"Server/TermLogger.hpp"
#include"Server/send_signal.hpp"

namespace Server {

int Stop::operator()(std::vector<std::string> params) {
	auto logger = Server::TermLogger();
	auto options = Server::OptionsHandler
		( logger
		, "cldcb-server stop [options]"
		, Server::OptionsHandler::ServerdirPidfile
		);
	auto optret = options.handle(params);
	if (optret)
		return *optret;

	if (!params.empty()) {
		std::cerr << "**BROKEN** Unexpected paramter to stop method."
			  << std::endl
			   ;
		return 1;
	}

	if (!Server::send_signal(options.pidfile(), SIGINT)) {
		std::cerr << "**BROKEN** Could not stop server."
			  << std::endl
			   ;
		return 1;
	}

	return 0;
}

}

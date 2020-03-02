#include<iostream>
#include<signal.h>
#include"Server/Stop.hpp"
#include"Server/send_signal.hpp"

namespace Server {

int Stop::operator()(std::vector<std::string> params) {
	/* TODO: check params.  */

	if (!params.empty()) {
		std::cerr << "**BROKEN** Unexpected paramter to stop method."
			  << std::endl
			   ;
		return 1;
	}

	if (!Server::send_signal(SIGINT)) {
		std::cerr << "**BROKEN** Could not stop server."
			  << std::endl
			   ;
		return 1;
	}

	return 0;
}

}

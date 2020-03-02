#include<iostream>
#include"Server/TermLogger.hpp"

namespace Server {

void TermLogger::log(Util::Logger::LogLevel l, std::string msg) {
	if (l == Util::Logger::Debug && !show_debug)
		return;
	switch (l) {
	case Util::Logger::Debug:
		std::cout << "(debug) " << msg << std::endl;
		break;
	case Util::Logger::Info:
		std::cout << msg << std::endl;
		break;
	case Util::Logger::Unusual:
		std::cerr << "(UNUSUAL) " << msg << std::endl;
		break;
	case Util::Logger::Broken:
		std::cerr << "**BROKEN** " << msg << std::endl;
		break;
	}
}

}


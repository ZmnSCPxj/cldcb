#include"Util/TermLogger.hpp"

namespace Util {

void TermLogger::log(Util::Logger::LogLevel l, std::string msg) {
	if (l == Util::Logger::Debug && !show_debug)
		return;
	switch (l) {
	case Util::Logger::Debug:
		cout << "(debug) " << msg << std::endl;
		break;
	case Util::Logger::Info:
		cout << msg << std::endl;
		break;
	case Util::Logger::Unusual:
		cerr << "(UNUSUAL) " << msg << std::endl;
		break;
	case Util::Logger::Broken:
		cerr << "**BROKEN** " << msg << std::endl;
		break;
	}
}

}


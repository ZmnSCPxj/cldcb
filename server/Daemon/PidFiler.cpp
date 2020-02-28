#include<fstream>
#include<iostream>
#include<sys/types.h>
#include<unistd.h>
#include"Daemon/PidFiler.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

PidFiler::PidFiler( Util::Logger& logger_
		  , std::string pid_path_
		  ) : logger(logger_)
		    , pid_path(std::move(pid_path_))
		    {
	auto os = Util::make_unique<std::ofstream>();
	os->exceptions(std::ios::failbit | std::ios::badbit);
	try {
		os->open(pid_path, std::ios::trunc);
	} catch (std::system_error const& e) {
		logger.BROKEN( "Failed to open: %s: %s"
			     , pid_path.c_str()
			     , e.code().message()
			     );
		std::cerr << "PidFiler: Failed to open: "
			  << pid_path << ": "
			  << e.code().message()
			   ;
		throw e;
	}
	*os << getpid() << std::endl;
	os = nullptr;
	logger.debug( "Wrote PID %d to: %s"
		    , (int) getpid()
		    , pid_path.c_str()
		    );
}

PidFiler::~PidFiler() {
	unlink(pid_path.c_str());
}

}


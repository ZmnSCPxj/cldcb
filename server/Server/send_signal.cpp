#include<fstream>
#include<signal.h>
#include<sys/types.h>
#include"Server/send_signal.hpp"  

#ifdef HAVE_CONFIG_H
# include"config.h"
#endif

namespace {
pid_t get_server_pid(std::string const& pidfile) {
	auto is = std::ifstream(pidfile);
	if (!is || is.bad() || !is.good())
		return (pid_t)-1;
	auto ret = pid_t();
	is >> ret;
	return ret;
}

}

namespace Server {

bool send_signal(std::string const& pidfile, int sig) {
	auto server_pid = get_server_pid(pidfile);
	return server_pid >= 0 && kill(server_pid, sig) == 0;
}

}

#include<fstream>
#include<signal.h>
#include<sys/types.h>
#include"Server/send_signal.hpp"  

namespace {
pid_t get_server_pid() {
	auto is = std::ifstream("cldcb-server.pid");
	if (!is || is.bad() || !is.good())
		return (pid_t)-1;
	auto ret = pid_t();
	is >> ret;
	return ret;
}

}

namespace Server {

bool send_signal(int sig) {
	auto server_pid = get_server_pid();
	return server_pid >= 0 && kill(server_pid, sig) == 0;
}

}

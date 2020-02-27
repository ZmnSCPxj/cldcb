#ifndef CLDCB_SERVER_SERVER_DAEMON_HPP
#define CLDCB_SERVER_SERVER_DAEMON_HPP

#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace Server {

/* Implements the "daemon" method.  */
class Daemon {
private:
	class Impl;
	std::shared_ptr<Impl> pimpl;

public:
	Daemon();
	Daemon(Daemon&& o) =default;
	Daemon& operator=(Daemon&& o) =default;
	Daemon(Daemon const&) =default;
	Daemon& operator=(Daemon const& o) =default;

	int operator()(std::vector<std::string>);
};

}

#endif /* CLDCB_SERVER_SERVER_DAEMON_HPP */

#include<errno.h>
#include<iostream>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include"Util/fork_test.hpp"
#include"Util/make_unique.hpp"

namespace Util {

std::unique_ptr<int>
fork_test(std::function<void()> child) {
	auto pid = fork();
	if (pid < pid_t(0))
		return Util::make_unique<int>(1);
	if (pid == pid_t(0)) {
		child();
		return Util::make_unique<int>(0);
	}
	auto exit = int(0);
	auto res = waitpid(pid, &exit, 0);
	if (res < pid_t(0)) {
		std::cout << "Failed to wait: "
			  << strerror(errno)
			  << std::endl
			   ;
		return Util::make_unique<int>(1);
	}
	/* Child exited normally.  */
	if (WIFEXITED(exit) && WEXITSTATUS(exit) == 0)
		return nullptr;

	return Util::make_unique<int>(1);
}

}

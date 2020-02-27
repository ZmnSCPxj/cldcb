#include<errno.h>
#include<exception>
#include<fcntl.h>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include"daemonize.hpp"

namespace {

bool write_all(int fd, void const* data, size_t size) {
	for (;;) {
		auto res = write(fd, data, size);
		if (res < 0 && errno == EINTR)
			continue;
		if (res <= 0)
			return false;
		if (size_t(res) == size)
			return true;

		data = (void const*)((char const*)data + res);
		size -= (size_t)res;
	}
}

void redirect_to_devnull(int fd, int o_flags) {
	auto open_fd = open("/dev/null", o_flags);
	if (open_fd < 0) {
		std::cerr << "open: " << strerror(errno) << std::endl;
		exit(250);
	}
	auto dup_fd = dup2(open_fd, fd);
	if (dup_fd != fd) {
		std::cerr << "dup2: " << strerror(errno) << std::endl;
		exit(250);
	}
	close(open_fd);
}

}

int daemonize(std::function<void (std::function<void (int)>)> handler) {
	int pipes[2];
	auto exitcode = int();

	auto piperes = pipe(pipes);
	if (piperes < 0) {
		std::cerr << "pipe: " << strerror(errno) << std::endl;
		return 250;
	}

	auto forkres = fork();
	if (forkres < 0) {
		close(pipes[0]);
		close(pipes[1]);
		std::cerr << "fork: " << strerror(errno) << std::endl;
		return 250;
	}

	if (forkres == 0) {
		auto completed = false;
		auto complete_daemonize = [&completed, &pipes](int exitcode) {
			if (completed)
				return;
			completed = true;

			auto setsid_res = setsid();
			if (setsid_res < 0) {
				std::cerr << "setsid: " << strerror(errno)
					  << std::endl
					   ;
				exit(250);
			}

			redirect_to_devnull(STDIN_FILENO, O_RDONLY);
			redirect_to_devnull(STDOUT_FILENO, O_WRONLY);
			redirect_to_devnull(STDERR_FILENO, O_WRONLY);

			/* Signal parent.  */
			auto write_res = write_all( pipes[1]
						  , &exitcode
						  , sizeof(exitcode)
						  );
			if (!write_res) {
				std::cerr << "write: " << strerror(errno)
					  << std::endl
					   ;
				exit(250);
			}
			close(pipes[1]);
		};
		/* child. */
		close(pipes[0]);
		try {
			handler(complete_daemonize);
		} catch (std::exception const& e) {
			if (!completed) {
				std::cerr << "Uncaught exception: "
					  << e.what() << std::endl
					   ;
				complete_daemonize(250);
			}
		} catch (...) {
			if (!completed) {
				std::cerr << "Uncaught unknown exception."
					  << std::endl
					   ;
				complete_daemonize(250);
			}
		}
		exit(0);
	} else {
		/* parent. */
		close(pipes[1]);
		auto res = ssize_t();
		do {
			res = read(pipes[0], &exitcode, sizeof(exitcode));
		} while(res < 0 && errno == EINTR);
		close(pipes[0]);
		if (res < 0) {
			std::cerr << "read: " << strerror(errno) << std::endl;
			return 250;
		}
		if (res == sizeof(exitcode))
			return exitcode;

		/* Child exited; catch its exitcode instead.  */
		waitpid(forkres, &exitcode, 0);
		if (WIFEXITED(exitcode)) {
			exitcode = WEXITSTATUS(exitcode);
			/* Not normal, so if it exited with 0, transform to
			 * 1.
			 */
			if (exitcode == 0)
				exitcode = 1;
			return exitcode;
		} else {
			std::cerr << "Child daemon died by signal: "
				  << WTERMSIG(exitcode)
				  << std::endl
				   ;
			return 250;
		}
	}
}


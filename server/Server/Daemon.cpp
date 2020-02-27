#include<exception>
#include<iostream>
#include<unistd.h>
#include"Server/Daemon.hpp"
#include"daemonize.hpp"

namespace Server {

class Daemon::Impl {
private:
	std::vector<std::string> params;

	bool initialize() {
		/* TODO */
		return true;
	}
	void loop() {
		/* TODO */
		pause();
	}

	void run(std::function<void(int)> complete_daemonize) {
		auto ini_success = false;
		try {
			ini_success = initialize();
		} catch (std::exception const& e) {
			std::cerr << "cldcb-server: "
				  << "Uncaught exception during initialize: "
				  << e.what()
				  << std::endl
				   ;
			ini_success = false;
		} catch (...) {
			std::cerr << "cldcb-server: "
				  << "Uncaught unknown exception "
				  << "during initialize."
				  << std::endl
				   ;
			ini_success = false;
		}
		if (!ini_success) {
			complete_daemonize(1);
			return;
		}

		std::cout << "cldcb-server daemonizing." << std::endl;
		complete_daemonize(0);
		loop();
	}

public:
	int daemonize_and_run(std::vector<std::string> params_) {
		params = params_;
		return daemonize([this](std::function<void(int)> complete) {
			this->run(complete);
		});
	}
};

Daemon::Daemon() : pimpl() { }

int Daemon::operator()(std::vector<std::string> params) {
	pimpl = std::make_shared<Impl>();
	return pimpl->daemonize_and_run(std::move(params));
}

}

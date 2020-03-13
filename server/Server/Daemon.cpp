#include<exception>
#include<iostream>
#include"Backup/ConnectionLoop.hpp"
#include"Daemon/Main.hpp"
#include"Net/SocketFd.hpp"
#include"Server/Daemon.hpp"
#include"Server/Logger.hpp"
#include"Util/make_unique.hpp"
#include"daemonize.hpp"

namespace Server {

class Daemon::Impl {
private:
	std::vector<std::string> params;

	std::unique_ptr<Server::Logger> plogger;

	std::unique_ptr<::Daemon::Main> main;

	bool initialize() {
		/* TODO: get log path from options or something.  */
		plogger = Util::make_unique<Server::Logger>("debug.log");

		auto looper_constructor = []( Util::Logger& logger
					    , ::Daemon::Breaker& breaker
					    , ::Daemon::ClientList& clients
					    ) {
			/* TODO: Archive::StorageImpl.  */
			return Util::make_unique<Backup::ConnectionLoop>
				( logger
				, breaker
				);
		};

		/* TODO: get options from params.  */
		main = Util::make_unique<::Daemon::Main>( *plogger
							, 29735
							, "cldcb-server.pid"
							, looper_constructor
							);

		/* TODO: other inits */

		return true;
	}
	void loop() {
		main->run();
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
		plogger->info("cldcb-server is now a daemon.");
		plogger->info( "cldcb-server is Free Software "
			       "WITHOUT ANY WARRANTY."
			     );
		loop();

		/* Clean up.  */
		main = nullptr;
		plogger = nullptr;
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

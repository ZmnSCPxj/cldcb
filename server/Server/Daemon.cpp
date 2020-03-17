#include<exception>
#include<iostream>
#include"Archive/StorageImpl.hpp"
#include"Backup/ConnectionLoop.hpp"
#include"Daemon/Main.hpp"
#include"Ev/ThreadPool.hpp"
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

	std::unique_ptr<Ev::ThreadPool> threadpool;

	std::unique_ptr<::Daemon::Main> main;

	bool initialize() {
		/* TODO: get from params or something.  */
		auto logfile = std::string("debug.log");
		auto max_count = std::uint16_t(19999);
		auto bindport = int(29735);
		auto pidfile = std::string("cldcb-server.pid");

		plogger = Util::make_unique<Server::Logger>(logfile);

		threadpool = Util::make_unique<Ev::ThreadPool>();

		auto looper_constructor = [ this
					  , max_count
					  ]( Util::Logger& logger
					   , ::Daemon::Breaker& breaker
					   , ::Daemon::ClientList& clients
					   ) {
			auto storage = Util::make_unique<Archive::StorageImpl>
				( logger
				, clients
				, *threadpool
				, max_count
				);

			return Util::make_unique<Backup::ConnectionLoop>
				( logger
				, breaker
				, std::move(storage)
				);
		};

		main = Util::make_unique<::Daemon::Main>( *plogger
							, bindport
							, pidfile
							, looper_constructor
							);

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
		threadpool = nullptr;
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

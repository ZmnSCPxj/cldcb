#include<exception>
#include<iostream>
#include"Archive/StorageImpl.hpp"
#include"Backup/ConnectionLoop.hpp"
#include"Daemon/Main.hpp"
#include"Ev/ThreadPool.hpp"
#include"Net/SocketFd.hpp"
#include"Server/Daemon.hpp"
#include"Server/Logger.hpp"
#include"Server/OptionsHandler.hpp"
#include"Server/TermLogger.hpp"
#include"Util/make_unique.hpp"
#include"daemonize.hpp"

namespace Server {

class Daemon::Impl {
private:
	/* The termlogger is only used for options, otherwise the
	 * rest of the daemon uses the plogger.
	 */
	Server::TermLogger termlogger;
	Server::OptionsHandler options;

	std::unique_ptr<Server::Logger> plogger;

	std::unique_ptr<Ev::ThreadPool> threadpool;

	std::unique_ptr<::Daemon::Main> main;

	bool initialize() {
		auto logfile = options.logfile();
		auto max_count = options.max_count();
		auto bindport = options.port();
		auto pidfile = options.pidfile();

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
	Impl() : termlogger(), options(termlogger) { }

	int daemonize_and_run(std::vector<std::string> params) {
		auto optret = options.handle(params);
		if (optret)
			return *optret;

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

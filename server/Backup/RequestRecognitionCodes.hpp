#ifndef CLDCB_SERVER_BACKUP_REQUESTRECOGNITIONCODES_HPP
#define CLDCB_SERVER_BACKUP_REQUESTRECOGNITIONCODES_HPP

#include<functional>
#include<memory>
#include<utility>
#include"Backup/PingPongMessenger.hpp"

namespace Backup { class StorageIf; }
namespace Daemon { class Messenger; }
namespace Util { class Logger; }

namespace Backup {

class RequestRecognitionCodes
	: public std::enable_shared_from_this<RequestRecognitionCodes> {
private:
	Util::Logger& logger;
	Backup::PingPongMessenger messenger;
	Backup::StorageIf& storage;
	std::function<Ev::Io<int>()> enter_loop;

	double start_time;

	std::vector<std::vector<std::uint8_t>> codes;

public:
	RequestRecognitionCodes() =delete;
	RequestRecognitionCodes(RequestRecognitionCodes&&) =delete;
	RequestRecognitionCodes(RequestRecognitionCodes const&) =delete;

private:
	RequestRecognitionCodes( Util::Logger& logger_
			       , Daemon::Messenger& messenger_
			       , Backup::StorageIf& storage_
			       , std::function<Ev::Io<int>()> enter_loop_
			       ) : logger(logger_)
				 , messenger(messenger_)
				 , storage(storage_)
				 , enter_loop(std::move(enter_loop_))
				 , start_time(0.0)
				 , codes()
				 { }
public:
	static
	std::shared_ptr<RequestRecognitionCodes>
	create( Util::Logger& logger
	      , Daemon::Messenger& messenger
	      , Backup::StorageIf& storage
	      , std::function<Ev::Io<int>()> enter_loop
	      ) {
		/* Cannot use std::make_shared since constructor is
		 * private.
		 */
		return std::shared_ptr<RequestRecognitionCodes>(
			new RequestRecognitionCodes( logger
						   , messenger
						   , storage
						   , std::move(enter_loop)
						   )
		);
	}

	Ev::Io<int> run();
private:
	/* Actual running of this handler.  */
	Ev::Io<bool> core_run();
	/* Send the data in batches.  */
	Ev::Io<bool> send_loop();
};

}

#endif /* CLDCB_SERVER_BACKUP_REQUESTRECOGNITIONCODES_HPP */

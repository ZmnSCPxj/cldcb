#ifndef CLDCB_SERVER_BACKUP_REQUESTBACKUPDATA_HPP
#define CLDCB_SERVER_BACKUP_REQUESTBACKUPDATA_HPP

#include<functional>
#include<memory>
#include<utility>
#include"Backup/DataReader.hpp"
#include"Backup/PingPongMessenger.hpp"

namespace Backup { class DataStorage; }
namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace Backup {

class RequestBackupData
		: public std::enable_shared_from_this<RequestBackupData> {
private:
	Util::Logger& logger;
	Backup::PingPongMessenger messenger;
	Backup::DataStorage& storage;
	std::function<Ev::Io<int>()> enter_loop;

	std::unique_ptr<Backup::DataReader> reader;

	double start_time;

	RequestBackupData( Util::Logger& logger_
			 , Daemon::Messenger& messenger_
			 , Backup::DataStorage& storage_
			 , std::function<Ev::Io<int>()> enter_loop_
			 ) : logger(logger_)
			   , messenger(messenger_)
			   , storage(storage_)
			   , enter_loop(std::move(enter_loop_))
			   , reader()
			   , start_time()
			   { }

public:
	static
	std::shared_ptr<RequestBackupData>
	create( Util::Logger& logger
	      , Daemon::Messenger& messenger
	      , Backup::DataStorage& storage
	      , std::function<Ev::Io<int>()> enter_loop
	      ) {
		return std::shared_ptr<RequestBackupData>(
			new RequestBackupData( logger
					     , messenger
					     , storage
					     , std::move(enter_loop)
					     )
		);
	}

	Ev::Io<int> run(Secp256k1::PubKey const&);
private:
	Ev::Io<bool> reup_loop();
	Ev::Io<bool> incr_loop();
};

}

#endif /* CLDCB_SERVER_BACKUP_REQUESTBACKUPDATA_HPP */

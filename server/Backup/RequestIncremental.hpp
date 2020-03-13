#ifndef CLDCB_SERVER_BACKUP_REQUESTINCREMENTAL_HPP
#define CLDCB_SERVER_BACKUP_REQUESTINCREMENTAL_HPP

#include<cstdint>
#include<functional>
#include<memory>
#include<utility>
#include"Backup/PingPongMessenger.hpp"
#include"Ev/Join.hpp"

namespace Backup { class IncrementalStorage; }
namespace Backup { class ReuploadStorage; }
namespace Backup { class StorageIf; }
namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace Backup {

/* This handles a sub-sequence where the client has just
 * sent `request_incremental` message.
 */
class RequestIncremental
		: public std::enable_shared_from_this<RequestIncremental> {
private:
	Util::Logger& logger;
	Backup::PingPongMessenger messenger;
	Backup::StorageIf& storage;
	std::function<Ev::Io<int>()> enter_loop;

	std::unique_ptr<Backup::IncrementalStorage> incremental_storage;
	std::unique_ptr<Backup::ReuploadStorage> reupload_storage;

	std::unique_ptr<Ev::Join> joiner;

	double start_time;

	RequestIncremental( Util::Logger& logger_
			  , Daemon::Messenger& messenger_
			  , Backup::StorageIf& storage_
			  , std::function<Ev::Io<int>()> enter_loop_
			  );

public:
	RequestIncremental() =delete;
	~RequestIncremental();

	static
	std::shared_ptr<RequestIncremental>
	create( Util::Logger& logger
	      , Daemon::Messenger& messenger
	      , Backup::StorageIf& storage
	      , std::function<Ev::Io<int>()> enter_loop
	      ) {
		return std::shared_ptr<RequestIncremental>(
			new RequestIncremental( logger, messenger, storage
					      , std::move(enter_loop)
					      )
		);
	}

	Ev::Io<int> run( Secp256k1::PubKey const& cid
		       , std::uint32_t data_version
		       );
private:
	/* After getting an IncrementalStorage, check if we need a
	 * reupload.
	 */
	Ev::Io<int> check_need_reupload();
	/* Spun off in a concurrent task, inform the client to
	 * send a reupload message.
	 */
	Ev::Io<int> send_response_reupload();
	/* Get `incremental_chunk`s.  */
	Ev::Io<bool> read_incremental();
	/* Get `reupload_chunk`s */
	Ev::Io<bool> read_reupload();
	/* Send `acknowledge_upload`.  */
	Ev::Io<bool> send_acknowledge_upload();
	/* End the sequence.  */
	Ev::Io<int> end(bool result);

};

}

#endif /* CLDCB_SERVER_BACKUP_REQUESTINCREMENTAL_HPP */

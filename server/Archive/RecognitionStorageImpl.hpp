#ifndef CLDCSB_SERVER_ARCHIVE_RECGONITIONSTORAGEIMPL_HPP
#define CLDCSB_SERVER_ARCHIVE_RECGONITIONSTORAGEIMPL_HPP

#include<string>
#include<unordered_map>
#include"Backup/RecognitionStorage.hpp"
#include"Ev/Semaphore.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Daemon { class ClientAllow; }
namespace Ev { class ThreadPool; }
namespace Util { class Logger; }

namespace Archive {

class RecognitionStorageImpl : public Backup::RecognitionStorage {
private:
	Util::Logger& logger;
	Daemon::ClientAllow& clientlist;
	Ev::ThreadPool& threadpool;

	Ev::Semaphore mtx;
	std::string filename;
	std::unordered_map<Secp256k1::PubKey, std::vector<std::uint8_t>>
		codes;

	void load_codes();
	bool save_codes();

public:
	explicit
	RecognitionStorageImpl( Util::Logger& logger_
			      , Daemon::ClientAllow& clientlist_
			      , Ev::ThreadPool& threadpool_
			      , std::string filename_
			      ) : logger(logger_)
				, clientlist(clientlist_)
				, threadpool(threadpool_)
				, mtx()
				, filename(std::move(filename_))
				, codes()
				{ load_codes(); }

	Ev::Io<bool> give_recognition_code( Secp256k1::PubKey const&
					  , std::vector<std::uint8_t> const&
					  ) override;
	Ev::Io<std::vector<std::vector<std::uint8_t>>>
	request_recognition_codes() override;
};

}

#endif /* CLDCSB_SERVER_ARCHIVE_RECGONITIONSTORAGEIMPL_HPP */

#ifndef CLDCB_SERVER_ARCHIVE_DATASTORAGEIMPL_HPP
#define CLDCB_SERVER_ARCHIVE_DATASTORAGEIMPL_HPP

#include<memory>
#include"Archive/FooterJudge.hpp"
#include"Backup/DataStorage.hpp"

namespace Archive { class FileNamer; }
namespace Daemon { class ClientList; }
namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }

namespace Archive {

class DataStorageImpl : public Backup::DataStorage {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	Daemon::ClientList& clientlist;
	Archive::FooterJudge judge;
	std::unique_ptr<Archive::FileNamer> namer;

public:
	DataStorageImpl() =delete;
	DataStorageImpl(DataStorageImpl&&) =default;

	DataStorageImpl( Util::Logger& logger
		       , Ev::ThreadPool& threadpool
		       , Daemon::ClientList& clientlist
		       , std::uint16_t max_count
		       , std::unique_ptr<Archive::FileNamer> namer
		       );
	~DataStorageImpl();

	Ev::Io<std::unique_ptr<Backup::IncrementalStorage>>
	request_incremental( Secp256k1::PubKey const& cid
			   , std::uint32_t data_version
			   ) override;
	Ev::Io<std::unique_ptr<Backup::DataReader>>
	request_backup_data(Secp256k1::PubKey const& cid) override;
};

}

#endif /* CLDCB_SERVER_ARCHIVE_DATASTORAGEIMPL_HPP */

#ifndef CLDCB_SERVER_ARCHIVE_STORAGEIMPL_HPP
#define CLDCB_SERVER_ARCHIVE_STORAGEIMPL_HPP

#include"Archive/DataStorageImpl.hpp"
#include"Archive/FileNamer.hpp"
#include"Archive/RecognitionStorageImpl.hpp"
#include"Backup/StorageIf.hpp"
#include"Ev/Io.hpp"
#include"Util/make_unique.hpp"

namespace Archive {

class StorageImpl : public Backup::StorageIf {
private:
	Archive::DataStorageImpl data_storage;
	Archive::RecognitionStorageImpl recog_storage;

public:
	StorageImpl() =delete;

	StorageImpl( Util::Logger& logger
		   , Daemon::ClientAllow& clientlist
		   , Ev::ThreadPool& threadpool
		   , std::uint16_t max_count = 19999
		   , std::string recognition_filename = "recognition_codes"
		   , std::unique_ptr<Archive::FileNamer> namer =
			Util::make_unique<Archive::FileNamer::Default>()
		   ) : data_storage( logger
				   , threadpool
				   , clientlist
				   , max_count
				   , std::move(namer)
				   )
		     , recog_storage( logger
				    , clientlist
				    , threadpool
				    , recognition_filename
				    )
		     { }

	/* Backup::DataStorage.  */
	Ev::Io<std::unique_ptr<Backup::IncrementalStorage>>
	request_incremental( Secp256k1::PubKey const& cid
			   , std::uint32_t data_version
			   ) override {
		return data_storage.request_incremental(cid, data_version);
	}
	Ev::Io<std::unique_ptr<Backup::DataReader>>
	request_backup_data(Secp256k1::PubKey const& cid) override {
		return data_storage.request_backup_data(cid);
	}
	/* Backup::RecognitionStorage.  */
	Ev::Io<bool> give_recognition_code( Secp256k1::PubKey const& cid
					  , std::vector<std::uint8_t> const& code
					  ) override {
		return recog_storage.give_recognition_code(cid, code);
	}
	Ev::Io<std::vector<std::vector<std::uint8_t>>>
	request_recognition_codes() override {
		return recog_storage.request_recognition_codes();
	}
};

}

#endif /* CLDCB_SERVER_ARCHIVE_STORAGEIMPL_HPP */

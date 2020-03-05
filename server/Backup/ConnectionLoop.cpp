#include"Backup/ConnectionLoop.hpp"
#include"Backup/DataReader.hpp"
#include"Backup/IncrementalStorage.hpp"
#include"Backup/ServiceLoop.hpp"
#include"Backup/StorageIf.hpp"
#include"Ev/Io.hpp"
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"Util/make_unique.hpp"

namespace {

class NullStorageIf : public Backup::StorageIf {
public:
	/* RecognitionStorage.  */
	Ev::Io<bool> give_recognition_code( Secp256k1::PubKey const&
					  , std::vector<std::uint8_t> const&
					  ) override {
		return Ev::lift_io(true);
	}
	Ev::Io<std::vector<std::vector<std::uint8_t>>>
	request_recognition_codes() override {
		return Ev::lift_io( std::vector<std::vector<std::uint8_t>>()
				  );
	}

	/* DataStorage.  */
	Ev::Io<std::unique_ptr<Backup::IncrementalStorage>>
	request_incremental( Secp256k1::PubKey const& cid
			   , std::uint32_t data_version
			   ) override {
		return Ev::lift_io<std::unique_ptr<Backup::IncrementalStorage>>(nullptr);
	}
	Ev::Io<std::unique_ptr<Backup::DataReader>>
	request_backup_data(Secp256k1::PubKey const& cid) override {
		return Ev::lift_io<std::unique_ptr<Backup::DataReader>>(nullptr);
	}
};

}

namespace Backup {

ConnectionLoop::ConnectionLoop( Util::Logger& logger_
			      , Daemon::Breaker& breaker_
			      ) : logger(logger_)
				, breaker(breaker_)
				, storage(Util::make_unique<NullStorageIf>())
				{ }

ConnectionLoop::~ConnectionLoop() { }

std::function<Ev::Io<int>()>
ConnectionLoop::new_handshaked_connection( Net::SocketFd fd
					 , Noise::Encryptor enc
					 , Secp256k1::PubKey const& cid
					 ) {
	auto serviceloop = Backup::ServiceLoop::create(logger, breaker
						      , std::move(fd)
						      , std::move(enc)
						      , cid
						      , *storage
						      );
	return [serviceloop]() {
		return serviceloop->enter_loop();
	};
}

}

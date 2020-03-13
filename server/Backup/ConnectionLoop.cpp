#include"Backup/ConnectionLoop.hpp"
#include"Backup/DataReader.hpp"
#include"Backup/IncrementalStorage.hpp"
#include"Backup/ServiceLoop.hpp"
#include"Backup/StorageIf.hpp"
#include"Ev/Io.hpp"
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"Util/make_unique.hpp"

namespace Backup {

ConnectionLoop::ConnectionLoop( Util::Logger& logger_
			      , Daemon::Breaker& breaker_
			      , std::unique_ptr<Backup::StorageIf> storage_
			      ) : logger(logger_)
				, breaker(breaker_)
				, storage(std::move(storage_))
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

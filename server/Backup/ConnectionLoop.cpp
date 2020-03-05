#include"Ev/Io.hpp"
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"Backup/ConnectionLoop.hpp"
#include"Backup/ServiceLoop.hpp"

namespace Backup {

std::function<Ev::Io<int>()>
ConnectionLoop::new_handshaked_connection( Net::SocketFd fd
					 , Noise::Encryptor enc
					 , Secp256k1::PubKey const& cid
					 ) {
	auto serviceloop = Backup::ServiceLoop::create(logger, breaker
						      , std::move(fd)
						      , std::move(enc)
						      , cid
						      );
	return [serviceloop]() {
		return serviceloop->enter_loop();
	};
}

}

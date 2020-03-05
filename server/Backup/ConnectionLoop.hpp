#ifndef CLDCB_SERVER_BACKUP_CONNECTIONLOOP_HPP
#define CLDCB_SERVER_BACKUP_CONNECTIONLOOP_HPP

#include<memory>
#include"Daemon/ConnectionLoop.hpp"

namespace Backup { class StorageIf; }
namespace Daemon { class Breaker; }
namespace Util { class Logger; }

namespace Backup {

class ConnectionLoop : public Daemon::ConnectionLoop {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;
	std::unique_ptr<Backup::StorageIf> storage;

public:
	ConnectionLoop( Util::Logger& logger_
		      , Daemon::Breaker& breaker_
		      );
	~ConnectionLoop();

	std::function<Ev::Io<int>()>
	new_handshaked_connection( Net::SocketFd
				 , Noise::Encryptor
				 , Secp256k1::PubKey const&
				 ) override;
};

}

#endif /* CLDCB_SERVER_BACKUP_CONNECTIONLOOP_HPP */

#include<sstream>
#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Daemon/Connection.hpp"
#include"Daemon/ConnectionHandshaker.hpp"
#include"Ev/Io.hpp"
#include"Noise/Encryptor.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

Connection::Connection( Util::Logger& logger_
		      , Daemon::Breaker& breaker_
		      , Secp256k1::KeyPair const& identity_
		      , std::string const& prologue_
		      , Net::SocketFd fd_
		      ) : logger(logger_)
			, breaker(breaker_)
			, fd(std::move(fd_))
			, handshaker(Util::make_unique<ConnectionHandshaker>
				( logger
				, breaker
				, identity_
				, prologue_
				, fd
				))
			{ }

Connection::~Connection() { }

Ev::Io<int> Connection::new_connection(std::shared_ptr<Connection> self) {
	return self->handshaker->handshake()
	     .then<int>([self](std::unique_ptr<std::pair<Noise::Encryptor, Secp256k1::PubKey>> handshake_result) {
		if (!handshake_result) {
			self->logger.info( "Handshake failure, "
					   "disconnecting <fd %d>"
					 , self->fd.get()
					 );
			return Ev::lift_io(0);
		}

		self->handshaker = nullptr;

		auto encryptor = std::move(handshake_result->first);
		auto incoming_id = std::move(handshake_result->second);

		{
			auto os = std::ostringstream();
			os << incoming_id;
			self->logger.info( "<fd %d> is %s"
					 , self->fd.get()
					 , os.str().c_str()
					 );
		}

		/* TODO.  */
		(void) encryptor;
		return Ev::lift_io(0);
	});
}

}

#include<sstream>
#include<unistd.h>
#include"Daemon/Breaker.hpp"
#include"Daemon/Connection.hpp"
#include"Daemon/ConnectionHandshaker.hpp"
#include"Daemon/ConnectionLoop.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
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
		      , Daemon::ConnectionLoop& looper_
		      ) : logger(logger_)
			, breaker(breaker_)
			, fd(std::move(fd_))
			, looper(looper_)
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

		auto fd_num = self->fd.get();
		auto ret = self->looper
			 .new_handshaked_connection( std::move(self->fd)
						   , std::move(encryptor)
						   , incoming_id
						   )
			 ;
		if (!ret) {
			self->logger.info( "Connection rejected after "
					   "handshake <fd %d>"
					 , fd_num
					 );
			return Ev::lift_io(0);
		}
		return Ev::yield().then<int>([ret](int) {
			return ret();
		});
	});
}

}

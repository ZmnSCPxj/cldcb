#include<utility>
#include"Daemon/Breaker.hpp"
#include"Daemon/ConnectionHandshaker.hpp"
#include"Daemon/IoResult.hpp"
#include"Ev/Io.hpp"
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

Secp256k1::KeyPair random_keypair() {
	Secp256k1::Random rand;
	return Secp256k1::KeyPair(rand);
}

}

namespace Daemon {

ConnectionHandshaker::ConnectionHandshaker( Util::Logger& logger_
					  , Daemon::Breaker& breaker_
					  , Secp256k1::KeyPair const& identity_
					  , std::string const& prologue_
					  , Net::SocketFd const& fd_
					  ) : logger(logger_)
					    , breaker(breaker_)
					    , responder( identity_
						       , random_keypair()
						       , prologue_
						       )
					    , fd(fd_)
					    { }

Ev::Io<std::unique_ptr<std::pair<Noise::Encryptor, Secp256k1::PubKey>>>
ConnectionHandshaker::handshake() {
	typedef std::pair<Noise::Encryptor, Secp256k1::PubKey> PairT;
	typedef std::unique_ptr<std::pair<Noise::Encryptor, Secp256k1::PubKey>> RetT;
	return breaker.read_timed( fd.get()
				 , Noise::Responder::act1_size
				 , 5.0 /* seconds timeout*/
				 )
	      .then<RetT>([this](Daemon::IoResult ior) {
		if (ior.data.size() != Noise::Responder::act1_size) {
			logger.info( "Could not get complete %d bytes act1 "
				     "on <fd %d>"
				   , (int)Noise::Responder::act1_size
				   , fd.get()
				   );
			return Ev::lift_io<RetT>(nullptr);
		}
		auto act2 = responder.act1_and_2(ior.data);
		if (!act2) {
			logger.info( "Handshake failed at act1 <fd %d>."
				   , fd.get()
				   );
			return Ev::lift_io<RetT>(nullptr);
		}
		return breaker.write_timed( fd.get()
					  , std::move(*act2)
					  , 5.0
					  )
		     .then<RetT>([this](Daemon::IoResult ior) {
			if (ior.data.size() != Noise::Responder::act3_size) {
				logger.info( "Could not get complete %d bytes "
					     "act3 on <fd %d>"
					   , (int)Noise::Responder::act3_size
					   , fd.get()
					   );
				return Ev::lift_io<RetT>(nullptr);
			}
			auto incoming_id = responder.act3(ior.data);
			if (!incoming_id) {
				logger.info( "Handshake failed at act 3 "
					     "<fd %d>"
					   , fd.get()
					   );
				return Ev::lift_io<RetT>(nullptr);
			}
			auto encryptor = responder.get_encryptor();
			auto pair = std::make_pair( std::move(encryptor)
						  , std::move(*incoming_id)
						  );
			auto ptr = Util::make_unique<PairT>(std::move(pair));
			return Ev::lift_io<RetT>(std::move(ptr));
		});
	});
}

}

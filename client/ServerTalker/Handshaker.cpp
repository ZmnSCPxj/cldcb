#include<errno.h>
#include<string.h>
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"
#include"ServerTalker/Handshaker.hpp"
#include"ServerTalker/rw.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

Secp256k1::KeyPair random_ephemeral() {
	Secp256k1::Random rand;
	return Secp256k1::KeyPair(rand);
}

}

namespace ServerTalker {

Handshaker::Handshaker( Util::Logger& logger_
		      , Secp256k1::KeyPair const& identity
		      , Secp256k1::PubKey const& server_identity
		      , std::string const& prologue
		      , Net::SocketFd const& fd_
		      ) : logger(logger_)
			, initiator( identity
				   , server_identity
				   , random_ephemeral()
				   , prologue
				   )
			, fd(fd_)
			{ }

std::unique_ptr<Noise::Encryptor> Handshaker::handshake() {
	auto act1 = initiator.act1();
	if (!write_all(fd.get(), &act1[0], act1.size())) {
		auto my_errno = errno;
		logger.unusual( "Could not write act1 on <fd %d>: %s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		errno = my_errno;
		return nullptr;
	}

	auto act2 = std::vector<std::uint8_t>(Noise::Initiator::act2_size);
	auto rdsize = act2.size();
	if (!read_all(fd.get(), &act2[0], rdsize)) {
		auto my_errno = errno;
		logger.unusual( "Could not read act2 on <fd %d>, "
				"only read %d: %s"
			      , fd.get()
			      , (int)rdsize
			      , strerror(my_errno)
			      );
		errno = my_errno;
		return nullptr;
	}

	auto pact3 = initiator.act2_and_3(std::move(act2));
	if (!pact3) {
		logger.unusual( "Invalid act2 <fd %d>.", fd.get());
		errno = EBADMSG;
		return nullptr;
	}
	auto& act3 = *pact3;
	if (!write_all(fd.get(), &act3[0], act3.size())) {
		auto my_errno = errno;
		logger.unusual( "Could not write act3 on <fd %d>: %s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		errno = my_errno;
		return nullptr;
	}

	return Util::make_unique<Noise::Encryptor>(initiator.get_encryptor());
}

}

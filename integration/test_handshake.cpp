#ifdef NDEBUG
#undef NDEBUG
#endif
#include<assert.h>
#include<iostream>
#include<thread>
#include"Daemon/Breaker.hpp"
#include"Daemon/ConnectionHandshaker.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Net/SocketFd.hpp"
#include"Net/make_nonblocking.hpp"
#include"Net/socketpair.hpp"
#include"Noise/Encryptor.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"ServerTalker/Handshaker.hpp"
#include"Util/make_unique.hpp"

int main() {
	Secp256k1::Random rand;
	auto logger = Server::TermLogger();
	auto breaker = Daemon::Breaker::initialize(logger);

	auto sid = Secp256k1::KeyPair(rand);
	auto cid = Secp256k1::KeyPair(rand);

	auto socks = Net::socketpair();
	auto& ssock = socks.first;
	auto& csock = socks.second;

	/* These will be filled later.  */
	auto senc = std::unique_ptr<Noise::Encryptor>();
	auto cenc = std::unique_ptr<Noise::Encryptor>();

	auto client = std::thread([&logger, &csock, &cid, &sid, &cenc]() {
		ServerTalker::Handshaker handshaker
			( logger
			, cid
			, sid.pub()
			, "CLDCB"
			, csock
			);
		cenc = handshaker.handshake();
		assert(cenc);
		csock.reset();
	});

	auto shandshaker = Daemon::ConnectionHandshaker
		( logger
		, *breaker
		, sid
		, "CLDCB"
		, ssock
		);

	auto rv = Ev::start( shandshaker.handshake()
			   .then<int>([ &ssock
				      , &cid
				      , &senc
				      ](std::unique_ptr<std::pair<Noise::Encryptor, Secp256k1::PubKey>> result) {
		assert(result);
		assert(result->second == cid.pub());
		senc = Util::make_unique<Noise::Encryptor>(
			std::move(result->first)
		);
		ssock.reset();
		return Ev::lift_io(0);
	}));

	client.join();

	assert(cenc);
	assert(senc);
	assert(cenc->get_rk() == senc->get_sk());
	assert(cenc->get_sk() == senc->get_rk());

	return 0;
}

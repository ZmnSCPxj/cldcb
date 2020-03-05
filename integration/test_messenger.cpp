#include<assert.h>
#include<thread>
#include<utility>
#include"Crypto/Secret.hpp"
#include"Daemon/Breaker.hpp"
#include"Daemon/Messenger.hpp"
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Net/SocketFd.hpp"
#include"Net/socketpair.hpp"
#include"Noise/Encryptor.hpp"
#include"Protocol/Message.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"ServerTalker/Messenger.hpp"
#include"Util/Str.hpp"
#include"Util/make_unique.hpp"

namespace {

Protocol::Message make_message() {
	Secp256k1::Random rand;
	auto ret = Protocol::Message();
	ret.id = std::uint16_t(rand.get()) + (std::uint16_t(rand.get()) << 8);
	auto ntlvs = rand.get() % 16;
	for (; ntlvs; --ntlvs) {
		auto t = rand.get();
		auto l = std::uint16_t(rand.get())
		       + (std::uint16_t(rand.get() % 4) << 8)
		       ;
		auto v = std::vector<std::uint8_t>(l);
		for (auto& b : v)
			b = rand.get();
		ret.tlvs.insert(std::make_pair(t, v));
	}
	/* Serialization inserts a tlv 255, so insert it as well here.  */
	if (ret.tlvs.find(255) == ret.tlvs.end()) {
		ret.tlvs.insert(std::make_pair( 255
					      , std::vector<std::uint8_t>()
					      ));
	}
	return ret;
}

/* Number of times to do send/receive.  */
auto constexpr num = std::size_t(501);

/* Models an echo server. */
class EchoServer {
private:
	Daemon::Messenger messenger;
	std::size_t count;
	bool timedout;

public:
	explicit EchoServer( Daemon::Messenger&& messenger_
			   ) : messenger(std::move(messenger_))
			     , count(0)
			     { }

	Ev::Io<int> loop() {
		if (count < num) {
			return messenger.receive_message(0.1, timedout)
			     .then<int>([this](std::unique_ptr<Protocol::Message> msg) {
				assert(!timedout);
				assert(msg);
				return messenger.send_message(std::move(*msg))
				     .then<int>([this](bool res) {
					assert(res);
					++count;
					return loop();
				});
			});
		} else
			return Ev::lift_io(0);
	}
};

}

int main() {
	Secp256k1::Random rand;
	auto logger = Server::TermLogger();

	auto s2c = Crypto::Secret(rand);
	auto c2s = Crypto::Secret(rand);
	auto ck = Crypto::Secret(rand);

	auto senc = ([&s2c, &c2s, &ck]() {
		auto r_cipherstate = Noise::Detail::CipherState();
		r_cipherstate.initialize_key(c2s);
		auto s_cipherstate = Noise::Detail::CipherState();
		s_cipherstate.initialize_key(s2c);
		return Noise::Encryptor( std::move(r_cipherstate)
				       , std::move(s_cipherstate)
				       , ck
				       );
	})();
	auto cenc = ([&s2c, &c2s, &ck]() {
		auto r_cipherstate = Noise::Detail::CipherState();
		r_cipherstate.initialize_key(s2c);
		auto s_cipherstate = Noise::Detail::CipherState();
		s_cipherstate.initialize_key(c2s);
		return Noise::Encryptor( std::move(r_cipherstate)
				       , std::move(s_cipherstate)
				       , ck
				       );
	})();
	assert(senc.get_rk() == cenc.get_sk());
	assert(senc.get_sk() == cenc.get_rk());
	
	auto socks = Net::socketpair();
	auto& csock = socks.first;
	auto& ssock = socks.second;

	auto client = std::thread([&logger, &cenc, &csock]() {
		auto messenger = ServerTalker::Messenger( logger
							, std::move(csock)
							, std::move(cenc)
							);
		for (auto i = 0; i < num; ++i) {
			auto msg = make_message();
			auto res = messenger.send_message(msg);
			assert(res);
			auto new_msg = messenger.receive_message();
			assert(new_msg);
			assert(msg.id == new_msg->id);
			assert(msg.tlvs == new_msg->tlvs);
		}
	});

	auto breaker = Daemon::Breaker::initialize(logger);
	auto messenger = Daemon::Messenger( logger
					  , *breaker
					  , std::move(ssock)
					  , std::move(senc)
					  );
	auto server = Util::make_unique<EchoServer>(std::move(messenger));

	Ev::start(server->loop());

	/* Clean up.  */
	server = nullptr;
	client.join();

	return 0;
}

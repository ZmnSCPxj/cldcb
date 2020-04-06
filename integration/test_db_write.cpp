#include<assert.h>
#include<stdlib.h>
#include<thread>
#include<unistd.h>
#include<vector>
#include"Archive/StorageImpl.hpp"
#include"Archive/FileNamer.hpp"
#include"Backup/ConnectionLoop.hpp"
#include"Crypto/Secret.hpp"
#include"Daemon/ClientAllow.hpp"
#include"Daemon/Breaker.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/start.hpp"
#include"LD/DbWrite.hpp"
#include"Net/SocketFd.hpp"
#include"Net/socketpair.hpp"
#include"Noise/Encryptor.hpp"
#include"Plugin/DbFileReader.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/Setup.hpp"
#include"Plugin/Single/Server.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"ServerTalker/Messenger.hpp"
#include"Util/Rw.hpp"
#include"Util/make_unique.hpp"

#include<iostream>

/* Test that simulates a few db_write events.  */

/* ClientAllow that allows everything.  */
class NullClientAllow : public Daemon::ClientAllow {
public:
	bool has(Secp256k1::PubKey const&) const { return true; }
};
/* FileNamer that gives /tmp/ filenames.  */
class TmpFileNamer : public Archive::FileNamer {
private:
	std::string archive_filename;
	std::string reupload_filename;
	std::string incremental_filename;

public:
	TmpFileNamer() : archive_filename("/tmp/dw.arch.XXXXXX")
		       , reupload_filename("/tmp/dw.reup.XXXXXX")
		       , incremental_filename("/tmp/dw.incr.XXXXXX")
		       {
		archive_filename += '\0';
		reupload_filename += '\0';
		incremental_filename += '\0';

		auto fd0 = mkstemp(&archive_filename[0]);
		assert(fd0 >= 0);
		close(fd0);
		auto fd1 = mkstemp(&reupload_filename[0]);
		assert(fd1 >= 0);
		close(fd1);
		auto fd2 = mkstemp(&incremental_filename[0]);
		assert(fd2 >= 0);
		close(fd2);
	}
	~TmpFileNamer() {
		unlink(archive_filename.c_str());
		unlink(reupload_filename.c_str());
		unlink(incremental_filename.c_str());
		errno = 0;
	}

	std::string
	get_archive_filename(Secp256k1::PubKey const&) const {
		return archive_filename;
	}
	std::string
	get_temp_reupload_filename(Secp256k1::PubKey const&) const {
		return reupload_filename;
	}
	std::string
	get_temp_incremental_filename(Secp256k1::PubKey const&) const {
		return incremental_filename;
	}
};

/* Create and fill p a faked database.  */
std::string create_db_file(Secp256k1::Random& rand) {
	auto db_filename = std::string("/tmp/dw_db.XXXXXX");
	db_filename += '\0';
	auto db_fd = mkstemp(&db_filename[0]);
	assert(db_fd >= 0);

	auto size = std::size_t(99999)
		  + (std::size_t(rand.get()) << 8)
		  + (std::size_t(rand.get()) << 0)
		  ;
	auto buff = std::vector<std::uint8_t>(size);
	for (auto& b : buff)
		b = rand.get();

	auto write_res = Util::Rw::write_all(db_fd, &buff[0], size);
	assert(write_res);

	auto close_res = close(db_fd);
	assert(close_res == 0);

	return db_filename;
}

int main() {
	Secp256k1::Random rand;
	Server::TermLogger logger;

	/* Generate keypairs.  */
	Secp256k1::KeyPair node_keypair(rand);
	Secp256k1::KeyPair plugin_keypair(rand);
	Secp256k1::KeyPair server_keypair(rand);

	auto plugin_setup = Plugin::Setup();
	plugin_setup.node_id = node_keypair.pub();
	plugin_setup.our_id = plugin_keypair.pub();
	plugin_setup.our_priv_key = plugin_keypair.priv();
	/* Other parts of setup are not used.  */

	/* Generate communication secrets.  */
	auto c2s = Crypto::Secret(rand);
	auto s2c = Crypto::Secret(rand);
	auto ck = Crypto::Secret(rand);

	/* Generate socketpair.  */
	auto socketpair = Net::socketpair();
	auto& client_fd = socketpair.first;
	auto& server_fd = socketpair.second;
	/* Generte noise encryptors.  */
	auto client_enc = Noise::Encryptor(s2c, c2s, ck);
	auto server_enc = Noise::Encryptor(c2s, s2c, ck);

	Ev::ThreadPool threadpool;

	auto client = std::thread([ &logger
				  , &rand
				  , &plugin_setup
				  , &client_fd
				  , &client_enc
				  ]() {
		auto db_filename = create_db_file(rand);
		auto file_reader = Plugin::DbFileReader(db_filename);
		auto make_messenger = [&logger, &client_fd, &client_enc]() {
			return Util::make_unique<ServerTalker::Messenger>
				( logger
				, std::move(client_fd)
				, std::move(client_enc)
				);
		};

		Plugin::Single::Server server_if( logger
						, std::move(make_messenger)
						);

		auto handler = Plugin::DbWriteHandler( plugin_setup
						     , server_if
						     , file_reader
						     );
		auto data_version = std::uint16_t(rand.get());

		for (auto i = 0; i < 3; ++i) {
			auto dbw = LD::DbWrite();
			dbw.data_version = data_version;
			dbw.writes = { "This is a test"
				     , "A test this is"
				     };
			auto handle_res = handler.handle(dbw);
			assert(handle_res);
			++data_version;
		}

		unlink(db_filename.c_str());
		/* Client will disconnect here, which should close its
		 * end of the socket, which should cause the server
		 * loop to exit.
		 */
	});

	auto clientlist = NullClientAllow();
	auto namer = Util::make_unique<TmpFileNamer>();
	auto storage = Util::make_unique<Archive::StorageImpl>
		( logger
		, clientlist
		, threadpool
		, 19999
		, "/tmp/dw.rc.XXXXXX"
		, std::move(namer)
		);

	auto breaker = Daemon::Breaker::initialize(logger);
	Backup::ConnectionLoop serverloop( logger
					 , *breaker
					 , std::move(storage)
					 );

	auto conn = serverloop.new_handshaked_connection
		( std::move(server_fd)
		, std::move(server_enc)
		, plugin_keypair.pub()
		);

	Ev::start(conn());
	conn = nullptr;

	client.join();
	return 0;
}

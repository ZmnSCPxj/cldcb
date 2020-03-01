#include<cstdint>
#include<cstdlib>
#include<errno.h>
#include<fcntl.h>
#include<fstream>
#include<memory>
#include<sstream>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Daemon/KeyKeeper.hpp"
#include"Net/Fd.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Logger.hpp"
#include"Util/Str.hpp"
#include"Util/make_unique.hpp"

namespace {

void create_secret(Util::Logger& logger) {
	auto fd = Net::Fd();
	logger.debug("Creating secret.");
	do {
		fd.reset(open( "secret"
			     , O_WRONLY | O_CREAT
			     , 0400
			     ));
	} while (!fd && errno == EINTR);
	if (!fd) {
		logger.BROKEN( "Failed to create secret: %s"
			     , strerror(errno)
			     );
		return;
	}

	Secp256k1::Random rand;
	auto new_key = Secp256k1::PrivKey(rand);

	std::uint8_t buffer[32];
	new_key.to_buffer(buffer);
	auto p = &buffer[0];
	auto s = sizeof(buffer);
	do {
		auto res = ssize_t();
		do {
			res = write(fd.get(), p, s);
		} while (res < 0 && errno == EINTR);
		if (res < 0) {
			logger.BROKEN( "Failed to write to secret: %s"
				     , strerror(errno)
				     );
			fd.reset(-1);
			unlink("secret");
			return;
		}
		p += res;
		s -= std::size_t(res);
	} while (s > 0);
	fd.reset(-1);
	logger.debug("Generated new secret.");
}

}

namespace Daemon {

KeyKeeper::KeyKeeper(Util::Logger& logger) {
	auto fd = Net::Fd();
	auto create_loop = false;
	do {
		create_loop = false;
		errno = 0;
		do {
			fd.reset(open("secret", O_RDONLY));
		} while (!fd && errno == EINTR);
		if (!fd && errno == ENOENT) {
			create_secret(logger);
			create_loop = true;
		} else if (!fd) {
			auto msg = std::string(strerror(errno));
			logger.BROKEN( "Failed to open secret: %s"
				     , msg.c_str()
				     );
			throw std::runtime_error( "Failed to open secret: "
						+ msg
						);
		}
	} while (create_loop);

	std::uint8_t buffer[32];
	auto p = &buffer[0];
	auto s = sizeof(buffer);
	do {
		auto res = ssize_t();
		do {
			res = read(fd.get(), p, s);
		} while (res < 0 && errno == EINTR);
		if (res < 0) {
			auto msg = std::string(strerror(errno));
			logger.BROKEN("Failed to read secret: %s"
				     , msg.c_str()
				     );
			throw std::runtime_error("Failed to read secret: "
						+ msg
						);
		}
		p += res;
		s -= std::size_t(res);
	} while (s > 0);

	k = Secp256k1::PrivKey::from_buffer(buffer);

	try {
		auto sid = ([this]() {
			auto os = std::ostringstream();

			std::uint8_t id_buffer[33];
			get_server_id().to_buffer(id_buffer);
			id_buffer[0] |= 0x50;

			for (auto b : id_buffer) {
				os << Util::Str::hexbyte(b);
			}

			return os.str();
		})();

		auto os = std::ofstream("server_id", std::ios::trunc);
		os << "# sid = " << sid << std::endl;
		logger.debug( "server_id: # sid = %s"
			    , sid.c_str()
			    );
	} catch(...) {}
}

}

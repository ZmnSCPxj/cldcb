#include<errno.h>
#include<fcntl.h>
#include<stdexcept>
#include<string>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<utility>
#include"Secp256k1/Random.hpp"

namespace Secp256k1 {

/* TODO: Alternative sources of random bytes for different
 * non-Posix operating systems.
 */
#if 1

class Random::Impl {
private:
	int fd;
	int counter;
	std::uint8_t buffer[64];

public:
	Impl() {
		do {
			fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		} while (fd < 0 && errno == EINTR);

		if (fd < 0) {
			do {
				fd = open("/dev/random", O_RDONLY | O_CLOEXEC);
			} while (fd < 0 && errno == EINTR);
		}

		if (fd < 0) {
			auto e = errno;
			errno = 0;
			throw std::runtime_error(
				std::string("Secp256k1::Random: "
					    "While opening /dev/random: "
					   ) +
				strerror(e)
			);
		}

		counter = 64;
	}
	~Impl() {
		/* Ignore errors on closure.  */
		close(fd);
		errno = 0;
	}

	std::uint8_t get() {
		if (counter == 64) {
			auto load = buffer;
			do {
				ssize_t s;
				do {
					s = read(fd, load, counter);
				} while (s < 0 && errno == EINTR);
				if (s < 0) {
					auto e = errno;
					errno = 0;
					counter = 64;
					throw std::runtime_error(
						std::string("Secp256k1::Random: "
							    "While reading: "
							   ) +
						strerror(e)
					);
				} else if (s == 0) {
					counter = 64;
					throw std::runtime_error(
						"Secp256k1::Random: "
						"Unexpected end-of-file."
					);
				}

				counter -= s;
				load += s;
			} while(counter != 0);
		}

		return buffer[counter++];
	}
};

#endif

Random::Random() {
	pimpl.reset(new Impl());
}
Random::~Random() { }

std::uint8_t Random::get() {
	return pimpl->get();
}

}


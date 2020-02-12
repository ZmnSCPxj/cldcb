#ifndef CLDCB_CLIENT_SECP256K1_RANDOM_HPP
#define CLDCB_CLIENT_SECP256K1_RANDOM_HPP

#include<cstdint>
#include<memory>

namespace Secp256k1 {

/* A source of random data.  */
class Random {
private:
	class Impl;

	std::unique_ptr<Impl> pimpl;

public:
	Random();
	Random(Random&&) =delete;
	Random(Random const&) =delete;
	~Random();

	std::uint8_t get();
};

}

#endif /* CLDCB_CLIENT_SECP256K1_RANDOM_HPP */

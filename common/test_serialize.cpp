#include<assert.h>
#include"S.hpp"
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Sha256/Hash.hpp"

namespace {

template<typename T>
void test(T const& t) {
	auto out = std::queue<std::uint8_t>();
	S::serialize(out, t);

	auto in = std::queue<std::uint8_t>(out);
	T new_t;
	S::deserialize(in, new_t);

	assert(t == new_t);
}

}

int main() {
	Secp256k1::Random rand;

	{
		bool b = (rand.get() & 0x1) ? bool(true) : bool(false);
		test(b);
	}

	{
		std::uint8_t b = std::uint8_t(rand.get());
		test(b);
	}
	{
		std::int8_t b = std::int8_t(rand.get());
		test(b);
	}

	{
		std::uint16_t b = (std::uint16_t(rand.get()) << 8)
				+ std::uint16_t(rand.get())
				;
		test(b);
	}
	{
		std::int16_t b = (std::uint16_t(rand.get()) << 8)
			       + std::uint16_t(rand.get())
			       ;
		test(b);
	}

	{
		std::uint32_t b = (std::uint32_t(rand.get()) << 24)
				+ (std::uint32_t(rand.get()) << 16)
				+ (std::uint32_t(rand.get()) << 8)
				+ std::uint32_t(rand.get())
				;
		test(b);
	}
	{
		std::int32_t b = (std::uint32_t(rand.get()) << 24)
			       + (std::uint32_t(rand.get()) << 16)
			       + (std::uint32_t(rand.get()) << 8)
			       + std::uint32_t(rand.get())
			       ;
		test(b);
	}

	{
		std::uint64_t b = (std::uint64_t(rand.get()) << 56)
				+ (std::uint64_t(rand.get()) << 48)
				+ (std::uint64_t(rand.get()) << 40)
				+ (std::uint64_t(rand.get()) << 32)
				+ (std::uint64_t(rand.get()) << 24)
				+ (std::uint64_t(rand.get()) << 16)
				+ (std::uint64_t(rand.get()) << 8)
				+ std::uint64_t(rand.get())
				;
		test(b);
	}
	{
		std::int64_t b = (std::uint64_t(rand.get()) << 56)
			       + (std::uint64_t(rand.get()) << 48)
			       + (std::uint64_t(rand.get()) << 40)
			       + (std::uint64_t(rand.get()) << 32)
			       + (std::uint64_t(rand.get()) << 24)
			       + (std::uint64_t(rand.get()) << 16)
			       + (std::uint64_t(rand.get()) << 8)
			       + std::uint64_t(rand.get())
			       ;
		test(b);
	}

	{
		auto b = std::string("\vThe quick brown fox jumps over the lazy dog.\r\n");
		test(b);
	}
	{
		auto b = std::string("\0\001", 2);
		test(b);
	}

	{
		auto b = std::vector<std::string>{ "hello"
						 , "world"
						 };
		test(b);
	}

	{
		auto b = Secp256k1::G;
		test(b);
	}

	{
		std::uint8_t buffer[32];
		for (auto i = 0; i < 32; ++i)
			buffer[i] = rand.get();
		auto b = Sha256::Hash::from_buffer(buffer);
		test(b);
	}

	{
		auto b = Secp256k1::PrivKey(rand);
		test(b);
	}

	return 0;
}

#include<assert.h>
#include"Crypto/Secret.hpp"
#include"Secp256k1/Random.hpp"

int main() {
	Secp256k1::Random rand;

	assert(Crypto::Secret() == Crypto::Secret("0000000000000000000000000000000000000000000000000000000000000000"));

	auto a = Crypto::Secret(rand);
	auto b = Crypto::Secret(rand);
	assert(a != b); /* With very high probability.  */

	auto copy_a = a;
	assert(a == copy_a);
	auto move_copy_a = std::move(copy_a);
	assert(a == move_copy_a);

	copy_a = b;
	assert(b == copy_a);
	move_copy_a = std::move(copy_a);
	assert(b == move_copy_a);

	{
		std::uint8_t const buffer[32] =
			{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
			, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
			, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
			, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
			};
		assert( Crypto::Secret::from_buffer(buffer)
		     == Crypto::Secret("0001020304050607000102030405060700010203040506070001020304050607")
		      );
	}

	/* Erased destruction.  */
	{
		std::uint8_t buffer[sizeof(Crypto::Secret)];
		auto s = new(buffer) Crypto::Secret(rand);
		s->~Secret();
		for (auto i = 0; i < sizeof(Crypto::Secret); ++i)
			assert(buffer[i] == 0);
	}

	return 0;
}

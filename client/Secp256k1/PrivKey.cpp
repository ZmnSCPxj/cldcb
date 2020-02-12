#include<assert.h>
#include<secp256k1.h>
#include<stdexcept>
#include<string.h>
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"

namespace  Secp256k1 {

PrivKey::PrivKey( Secp256k1::Context const& ctx_
		, std::uint8_t key_[32]
		) : ctx(ctx_) {
	memcpy(key, key_, 32);
}

PrivKey::PrivKey( Secp256k1::Context const& ctx_
		, Secp256k1::Random& rand
		) : ctx(ctx_) {
	for (auto i = 0; i < 32; ++i) {
		key[i] = rand.get();
	}
}

PrivKey::PrivKey(PrivKey const& o) : ctx(o.ctx) {
	memcpy(key, o.key, 32);
}

PrivKey& PrivKey::negate() {
	auto res = secp256k1_ec_privkey_negate(ctx.get(), key);
	assert(res == 1);
	return *this;
}

PrivKey& PrivKey::operator+=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_add(ctx.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

PrivKey& PrivKey::operator*=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_mul(ctx.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

}

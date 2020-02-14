#include<assert.h>
#include<iomanip>
#include<secp256k1.h>
#include<sstream>
#include<stdexcept>
#include<string>
#include<string.h>
#include"Secp256k1/Detail/context.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"

using Secp256k1::Detail::context;

namespace  Secp256k1 {

PrivKey::PrivKey(std::uint8_t key_[32]) {
	memcpy(key, key_, 32);
}

PrivKey::PrivKey(Secp256k1::Random& rand) {
	for (auto i = 0; i < 32; ++i) {
		key[i] = rand.get();
	}
}

PrivKey::PrivKey(PrivKey const& o) {
	memcpy(key, o.key, 32);
}

PrivKey& PrivKey::negate() {
	auto res = secp256k1_ec_privkey_negate(context.get(), key);
	assert(res == 1);
	return *this;
}

PrivKey& PrivKey::operator+=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_add(context.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

PrivKey& PrivKey::operator*=(PrivKey const& o) {
	auto res = secp256k1_ec_privkey_tweak_mul(context.get(), key, o.key);
	/* FIXME: Use a backtrace-catching exception. */
	if (!res)
		throw std::out_of_range("Secp256k1::PrivKey += out-of-range");
	return *this;
}

}

namespace {

std::string hexbyte(std::uint8_t v) {
	std::ostringstream os;
	os << std::hex << std::setfill('0') << std::setw(2);
	os << ((unsigned int) v);
	return os.str();
}

}

std::ostream& operator<<(std::ostream& os, Secp256k1::PrivKey const& sk) {
	for (auto i = 0; i < 32; ++i)
		os << hexbyte(sk.key[i]);
	return os;
}

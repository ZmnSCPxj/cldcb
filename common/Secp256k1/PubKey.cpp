#include<assert.h>
#include<iomanip>
#include<secp256k1.h>
#include<sstream>
#include<string>
#include<string.h>
#include<utility>
#include"Secp256k1/Context.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Secp256k1 {

class PubKey::Impl {
public:
	Secp256k1::Context ctx;
	secp256k1_pubkey key;

	explicit Impl(Secp256k1::PrivKey const& sk) : ctx(sk.ctx) {
		auto res = secp256k1_ec_pubkey_create( ctx.get()
						     , &key
						     , sk.key
						     );
		if (!res)
			throw InvalidPrivKey();
	}
	explicit Impl(Secp256k1::Context ctx_) : ctx(ctx_) { }
	Impl(Impl const& o) : ctx(o.ctx) {
		memcpy(&key, &o.key, sizeof(key));
	}

	void negate() {
		auto res = secp256k1_ec_pubkey_negate(ctx.get(), &key);
		assert(res == 1);
	}
	void add(Impl const& o) {
		secp256k1_pubkey tmp;

		secp256k1_pubkey const* keys[2] = {&o.key, &key};
		auto res = secp256k1_ec_pubkey_combine( ctx.get()
						      , &tmp
						      , keys
						      , 2
						      );
		/* FIXME: use a backtrace-prserving exception.  */
		if (!res)
			throw std::out_of_range(
				"Secp256k1::PubKey::operatoor+=: "
				"result of adding PubKey out-of-range"
			);

		key = tmp;
	}
	void mul(Secp256k1::PrivKey const& sk) {
		/* Construct temporary. */
		auto tmp = key;

		auto res = secp256k1_ec_pubkey_tweak_mul( ctx.get()
							, &tmp
							, sk.key
							);
		if (!res)
			throw std::out_of_range(
				"Secp256k1::PubKey::operatoor*=: "
				"result of multiplying PrivKey out-of-range"
			);
		/* swap. */
		key = tmp;
	}

	bool equal(Impl const& o) const {
		std::uint8_t a[33];
		std::uint8_t b[33];
		size_t asize = sizeof(a);
		size_t bsize = sizeof(b);

		auto resa = secp256k1_ec_pubkey_serialize( ctx.get()
							 , a
							 , &asize
							 , &key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resa == 1);
		assert(asize == sizeof(a));

		auto resb = secp256k1_ec_pubkey_serialize( ctx.get()
							 , b
							 , &bsize
							 , &o.key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resb == 1);
		assert(bsize == sizeof(b));

		return memcmp(a, b, sizeof(a)) == 0;
	}

	std::string hexbyte(std::uint8_t v) {
		std::ostringstream os;
		os << std::hex << std::setfill('0') << std::setw(2);
		os << ((unsigned int) v);
		return os.str();
	}

	void dump(std::ostream& os) {
		std::uint8_t a[33];
		size_t asize = sizeof(a);

		auto resa = secp256k1_ec_pubkey_serialize( ctx.get()
							 , a
							 , &asize
							 , &key
							 , SECP256K1_EC_COMPRESSED
							 );
		assert(resa == 1);
		assert(asize == sizeof(a));

		for (auto i = 0; i < 33; ++i) {
			os << hexbyte(a[i]);
		}
	}
};

PubKey::PubKey(Secp256k1::PrivKey const& sk)
	/* FIXME: Use a make_unique factory.  */
	: pimpl(new Impl(sk)) {}

PubKey::PubKey(PubKey const& o)
	/* FIXME: Use a make_unique factory.  */
	: pimpl(new Impl(*o.pimpl)) { }
PubKey::PubKey(PubKey&& o) {
	/* FIXME: Use a make_unique factory.  */
	auto mine = std::unique_ptr<Impl>(new Impl(o.pimpl->ctx));
	std::swap(pimpl, mine);
	std::swap(pimpl, o.pimpl);
}
PubKey::~PubKey() { }

void PubKey::negate() {
	pimpl->negate();
}

PubKey& PubKey::operator+=(PubKey const& o) {
	pimpl->add(*o.pimpl);
	return *this;
}
PubKey& PubKey::operator*=(PrivKey const& o) {
	pimpl->mul(o);
	return *this;
}

bool PubKey::operator==(PubKey const& o) const {
	return pimpl->equal(*o.pimpl);
}

}

std::ostream& operator<<(std::ostream& os, Secp256k1::PubKey const& pk) {
	pk.pimpl->dump(os);
	return os;
}

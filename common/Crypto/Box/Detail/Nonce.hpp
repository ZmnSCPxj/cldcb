#ifndef CLDCB_COMMON_CRYPTO_BOX_DETAIL_NONCE_HPP
#define CLDCB_COMMON_CRYPTO_BOX_DETAIL_NONCE_HPP

#include<cstdint>
#include<stdexcept>

namespace Crypto { namespace Box { namespace Detail {

/* 96-bit nonce value.  */
class Nonce {
private:
	std::uint8_t nonce[12];

public:
	Nonce() {
		for (auto i = 0; i < 12; ++i)
			nonce[i] = 0;
	}
	Nonce(Nonce const&) =default;
	Nonce(Nonce&&) =default;
	Nonce& operator=(Nonce const&) =default;
	Nonce& operator=(Nonce&&) =default;

	Nonce& operator++() {
		for (auto i = 0; i < 12; ++i) {
			++nonce[i];
			/* Unsigned overflow is well-defined -- it is
			 * wraparound.
			 * If the nonce byte is not 0 after incrementing,
			 * just return now.
			 * If the nonce byte is 0 after incrementing,
			 * propagate the carry to the next loop iteration.
			 */
			if (nonce[i] != 0)
				return *this;
		}
		/* If we reached here, we wrapped around completely.
		 * Reset back to 0xfffff..., then throw.
		 */
		for (auto i = 0; i < 12; ++i)
			nonce[i] = 0xff;
		throw std::range_error("Crypto::Box::Detail::Nonce saturated, too large object to encrypt!");
	}

	std::uint8_t const* get() const { return nonce; }
};

}}}

#endif /* CLDCB_COMMON_CRYPTO_BOX_DETAIL_NONCE_HPP */

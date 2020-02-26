#ifndef CLDCB_COMMON_NOISE_DETAIL_HS_HPP
#define CLDCB_COMMON_NOISE_DETAIL_HS_HPP

#include<cstdlib>
#include<string>
#include<utility>
#include"Crypto/Secret.hpp"

namespace Noise { namespace Detail {

/* Common data passed throughout the handshake process.  */
class HS {
private:
	Crypto::Secret ck;
	Crypto::Secret h;

public:
	/* protocol_name is hashed first, but prologue is more likely to
	 * be changed.
	 */
	HS( std::string const& prologue = "lightning"
	  , std::string const& protocol_name = "Noise_XK_secp256k1_ChaChaPoly_SHA256"
	  );
	HS(HS const&) =default;
	HS(HS&&) =default;
	HS& operator=(HS const&) =default;
	HS& operator=(HS&&) =default;

	Crypto::Secret const& get_h() const { return h; }
	void mix_h(void const* p, std::size_t len);

	Crypto::Secret const& get_ck() const { return ck; }
	Crypto::Secret mix_ck(Crypto::Secret const&);
	std::pair<Crypto::Secret, Crypto::Secret> split_ck() const;

};

}}

#endif /* CLDCB_COMMON_NOISE_DETAIL_HS_HPP */

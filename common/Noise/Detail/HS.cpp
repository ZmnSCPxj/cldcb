#include<cstdint>
#include<cstring>
#include<vector>
#include"Noise/Detail/HS.hpp"
#include"Noise/Detail/hkdf2.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

namespace Noise { namespace Detail {

HS::HS( std::string const& prologue
      , std::string const& protocol_name
      ) {
	auto h_hash = Sha256::fun(protocol_name.c_str(), protocol_name.length());
	std::uint8_t buffer[32];
	h_hash.to_buffer(buffer);
	h = Crypto::Secret::from_buffer(buffer);

	ck = h;

	mix_h(prologue.c_str(), prologue.length());
}
void HS::mix_h(void const* p, std::size_t len) {
	auto buffer = std::vector<std::uint8_t>(32 + len);
	h.to_buffer(&buffer[0]);
	std::memcpy(&buffer[32], p, len);

	auto h_hash = Sha256::fun(&buffer[0], buffer.size());
	h_hash.to_buffer(&buffer[0]);
	h = Crypto::Secret::from_buffer(&buffer[0]);
}

Crypto::Secret HS::mix_ck(Crypto::Secret const& k) {
	auto res = Noise::Detail::hkdf2(ck, k);
	ck = res.first;
	return res.second;
}

std::pair<Crypto::Secret, Crypto::Secret>
HS::split_ck() const {
	return Noise::Detail::hkdf2_zero(ck);
}

}}

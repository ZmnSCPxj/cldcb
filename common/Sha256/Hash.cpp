#include<iomanip>
#include<sstream>
#include"Sha256/Hash.hpp"
#include"Util/Str.hpp"

std::ostream& operator<<(std::ostream& os, Sha256::Hash const& h) {
	std::uint8_t buffer[32];
	h.to_buffer(buffer);
	for (auto i = 0; i < 32; ++i)
		os << Util::Str::hexbyte(buffer[i]);
	return os;
}

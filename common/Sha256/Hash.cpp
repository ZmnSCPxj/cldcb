#include<iomanip>
#include<stdexcept>
#include<sstream>
#include"Sha256/Hash.hpp"
#include"Util/Str.hpp"

namespace Sha256 {

Hash::Hash(std::string const& s) {
	auto data = Util::Str::hexread(s);
	if (data.size() != 32)
		/* FIXME: Use a backtrace-catching exception.  */
		throw std::invalid_argument("Hash hex strings must be 32 bytes.");

	for (auto i = 0; i < 32; ++i)
		hash[i] = data[i];
}

}

std::ostream& operator<<(std::ostream& os, Sha256::Hash const& h) {
	std::uint8_t buffer[32];
	h.to_buffer(buffer);
	for (auto i = 0; i < 32; ++i)
		os << Util::Str::hexbyte(buffer[i]);
	return os;
}

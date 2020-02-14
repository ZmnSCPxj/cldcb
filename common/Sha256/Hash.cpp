#include<iomanip>
#include<sstream>
#include"Sha256/Hash.hpp"

namespace {

std::string hexbyte(std::uint8_t v) {
	std::ostringstream os;
	os << std::hex << std::setfill('0') << std::setw(2);
	os << ((unsigned int) v);
	return os.str();
}

}

std::ostream& operator<<(std::ostream& os, Sha256::Hash const& h) {
	std::uint8_t buffer[32];
	h.to_buffer(buffer);
	for (auto i = 0; i < 32; ++i)
		os << hexbyte(buffer[i]);
	return os;
}

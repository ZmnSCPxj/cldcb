#include<iomanip>
#include<sstream>
#include"common/Util/Str.hpp"

namespace Util {
namespace Str {

std::string hexbyte(std::uint8_t v) {
	std::ostringstream os;
	os << std::hex << std::setfill('0') << std::setw(2);
	/* uint8_t might be a char, which would confuse iostreams.
	 * Individual char is printed out as an ASCII character.
	 * Cast to an unsigned int to ensure it is printed as a
	 * number.
	 */
	os << ((unsigned int) v);
	return os.str();
}

}
}

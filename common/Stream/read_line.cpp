#include<sstream>
#include"Stream/read_line.hpp"

namespace Stream {

std::string read_line(std::istream& is) {
	auto os = std::ostringstream();
	auto c = char();

	while (is && !is.eof()) {
		is.read(&c, 1);
		if (is.eof())
			break;
		if (c == '\n')
			break;
		os << c;
	}

	return os.str();
}

}


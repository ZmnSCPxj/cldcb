#include<assert.h>
#include<sstream>
#include"Stream/read_line.hpp"

char const *testdata = R"(Line 1
Line 2

line4
line5)";

int main() {
	auto is = std::istringstream(testdata);

	assert(Stream::read_line(is) == "Line 1");
	assert(Stream::read_line(is) == "Line 2");
	assert(Stream::read_line(is) == "");
	assert(Stream::read_line(is) == "line4");
	assert(Stream::read_line(is) == "line5");

	return 0;
}

#include<algorithm>
#include"Plugin/OptFile/parse.hpp"
#include"Stream/read_line.hpp"
#include"Util/Str.hpp"

namespace Plugin { namespace OptFile {

std::map<std::string, std::string> parse(std::istream& is) {
	auto ret = std::map<std::string, std::string>();

	while (is && !is.eof()) {
		auto line = Util::Str::trim(Stream::read_line(is));
		if (line.size() <= 1)
			continue;
		if (line[0] != '#')
			continue;
		if (line[0] == '#' && (line[1] == '!' || line[1] == '#'))
			continue;

		/* Delete '#'.  */
		line.erase(line.begin());
		/* Find '='.  */
		auto equals = std::find(line.begin(), line.end(), '=');
		/* Extract key.  */
		auto key = Util::Str::trim(std::string(line.begin(), equals));
		/* Extract value.  */
		if (equals != line.end())
			++equals;
		auto value = Util::Str::trim(std::string(equals, line.end()));

		ret[std::move(key)] = std::move(value);
	}

	return ret;
}

}}

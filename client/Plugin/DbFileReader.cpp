#include<stdexcept>
#include"Plugin/DbFileReader.hpp"

namespace Plugin {

std::unique_ptr<DbFileReader::Session> DbFileReader::start() {
	/* Constructor is private, so Util::make_unique cannot access.  */
	auto ret = std::unique_ptr<DbFileReader::Session>(nullptr);
	ret.reset(new DbFileReader::Session(
		std::ifstream(path, std::ifstream::binary)
	));
	return ret;
}
std::vector<std::uint8_t>
DbFileReader::Session::read(unsigned int max_length) {
	if (file.eof())
		return std::vector<std::uint8_t>();
	if (!file)
		throw std::runtime_error("Unable to read database file!");

	auto ret = std::vector<std::uint8_t>(max_length);
	file.read((char*)&ret[0], max_length);
	ret.resize(file.gcount());
	return ret;
}

}

#include<sstream>
#include"Archive/FileNamer.hpp"
#include"Secp256k1/PubKey.hpp"

namespace {

std::string stringify_cid(Secp256k1::PubKey const& cid) {
	auto os = std::ostringstream();
	os << cid;
	auto str = os.str();
	str[0] = 'c';
	return str;
}

}

namespace Archive {

std::string
FileNamer::Default::get_archive_filename
		(Secp256k1::PubKey const& cid) const {
	return stringify_cid(cid) + ".arch";
}
std::string
FileNamer::Default::get_temp_reupload_filename
		(Secp256k1::PubKey const& cid) const {
	return stringify_cid(cid) + ".r.tmp";
}
std::string
FileNamer::Default::get_temp_incremental_filename
		(Secp256k1::PubKey const& cid) const {
	return stringify_cid(cid) + ".i.tmp";
}

}

#include<iostream>
#include<fstream>
#include"Plugin/OptFile/load.hpp"
#include"Plugin/OptFile/parse.hpp"
#include"Plugin/OptFile/translate_setup.hpp"
#include"Plugin/OptFile/validate_keys.hpp"
#include"Util/make_unique.hpp"

namespace Plugin { namespace OptFile {

std::unique_ptr<int> load(Plugin::Setup& setup, std::string filename) {
	auto file = Util::make_unique<std::ifstream>(filename);
	if (!file->good()) {
		std::cerr << "cldcb-plugin: Could not open file: "
			  << filename << std::endl
			   ;
		return Util::make_unique<int>(1);
	}

	auto options = Plugin::OptFile::parse(*file);
	file = nullptr;

	auto err1 = Plugin::OptFile::validate_keys(options);
	if (err1 != "") {
		std::cerr << "cldcb-plugin: " << err1 << std::endl;
		return Util::make_unique<int>(1);
	}

	auto err2 = Plugin::OptFile::translate_setup(setup, options);
	if (err2 != "") {
		std::cerr << "cldcb-plugin: " << err2 << std::endl;
		return Util::make_unique<int>(1);
	}

	return nullptr;
}

}}

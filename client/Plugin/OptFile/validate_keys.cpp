#include<vector>
#include<set>
#include"Plugin/OptFile/validate_keys.hpp"

namespace {

auto const valid_keys = std::set<std::string>
	{ "cid", "cpk"
	, "nid", "nsig"
	, "sid", "shost"
	, "proxy"
	};
auto const required_keys = std::vector<std::string>
	{ "cid", "cpk"
	, "nid", "nsig"
	, "sid", "shost"
	};

}

namespace Plugin { namespace OptFile {

std::string validate_keys(std::map<std::string, std::string> const& options) {
	for (auto const& e : options) {
		auto const& k = e.first;
		auto it = valid_keys.find(k);
		if (it == valid_keys.end())
			return "Invalid option: " + k;
	}

	for (auto const& k : required_keys) {
		auto it = options.find(k);
		if (it == options.end())
			return "Required option missing: " + k;
	}

	return "";
}

}}


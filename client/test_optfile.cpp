#include<assert.h>
#include<sstream>
#include"Plugin/ServerSpec.hpp"
#include"Plugin/Setup.hpp"

# include"Plugin/OptFile/parse.cpp"
# include"Plugin/OptFile/validate_keys.cpp"

auto parse_testdata = R"(#!/bin/sh
## A true comment

#	sid	= $SERVER
#nid=$NODE
	#cid = $CLIENT
#       justoption

## Ignore
#! Ignore2

exec cldcb-plugin -- "$0"
)";

auto missingkeys_testdata = R"(#!/bin/sh

# nid = x
# cid = y
# sid = z

)";

auto correct_testdata = R"(#!/bin/sh

# nid = x
# nsig = xx
# cid = y
# cpk = xx
# sid = z
# shost = zz
####
# proxy = wwwwww

)";

auto correct2_testdata = R"(#!/bin/sh

# nid = x
# nsig = xx
# cid = y
# cpk = xx
# sid = z
# shost = zz
####
#### proxy = wwwwww

)";

int main() {
	auto is = std::istringstream(parse_testdata);

	auto options = Plugin::OptFile::parse(is);

	assert(options.find("sid") != options.end());
	assert(options["sid"] == "$SERVER");

	assert(options.find("nid") != options.end());
	assert(options["nid"] == "$NODE");

	assert(options.find("cid") != options.end());
	assert(options["cid"] == "$CLIENT");

	assert(options.find("justoption") != options.end());
	assert(options["justoption"] == "");

	assert(options.find("Ignore") == options.end());
	assert(options.find("Ignore2") == options.end());

	/* justoption is not a valid key.  */
	assert(Plugin::OptFile::validate_keys(options) != "");

	{
		auto is = std::istringstream(missingkeys_testdata);
		auto options = Plugin::OptFile::parse(is);
		/* Missing options.  */
		assert(Plugin::OptFile::validate_keys(options) != "");
	}

	{
		auto is = std::istringstream(correct_testdata);
		auto options = Plugin::OptFile::parse(is);
		/* Kosher.  */
		assert(Plugin::OptFile::validate_keys(options) == "");
	}

	{
		auto is = std::istringstream(correct2_testdata);
		auto options = Plugin::OptFile::parse(is);
		/* Missing proxy, but not required option anyway.  */
		assert(Plugin::OptFile::validate_keys(options) == "");
	}

	return 0;
}

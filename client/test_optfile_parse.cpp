#include<assert.h>
#include<sstream>

# include"Plugin/OptFile/parse.cpp"

auto testdata = R"(#!/bin/sh
## A true comment

#	sid	= $SERVER
#nid=$NODE
	#cid = $CLIENT
#       justoption

## Ignore
#! Ignore2

exec cldcb-plugin -- "$0"
)";

int main() {
	auto is = std::istringstream(testdata);

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

	return 0;
}

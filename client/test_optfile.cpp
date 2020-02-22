#include<assert.h>
#include<sstream>
#include"Plugin/ServerSpec.hpp"
#include"Plugin/Setup.hpp"

# include"Plugin/OptFile/parse.cpp"
# include"Plugin/OptFile/translate_setup.cpp"
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

auto sample_testdata = R"(#!/bin/sh

# nid = 028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7
# nsig = 8d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f78d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7
# cid = C28d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7
# cpk = 1e2fb3c8fe8fb9f262f649f64d26ecf0f2c0a805a767cf02dc2d77a6ef1fdcc3
# proxy = 127.0.0.1:9150
# sid = 528d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7
# shost = [fe80::175c:26e7:55dd:e468]

exec cldcb-plugin -- "$0"
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

	{
		auto is = std::istringstream(sample_testdata);
		auto options = Plugin::OptFile::parse(is);
		assert(Plugin::OptFile::validate_keys(options) == "");
		auto setup = Plugin::Setup();
		assert(Plugin::OptFile::translate_setup(setup, options) == "");
		assert( setup.node_id
		     == Secp256k1::PubKey("028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7")
		      );
		assert( setup.our_id
		     == Secp256k1::PubKey("028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7")
		      );
		assert( setup.our_priv_key
		     == Secp256k1::PrivKey("1e2fb3c8fe8fb9f262f649f64d26ecf0f2c0a805a767cf02dc2d77a6ef1fdcc3")
		      );
		assert(setup.has_proxy);
		assert(setup.proxy_host == "127.0.0.1");
		assert(setup.proxy_port == 9150);
		assert(setup.servers.size() == 1);
		auto const& server = setup.servers[0];
		assert( server.id
		     == Secp256k1::PubKey("028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7")
		      );
		assert(server.host == "fe80::175c:26e7:55dd:e468");
		assert(server.port == 29735);
	}

	return 0;
}

#include<algorithm>
#include"Plugin/OptFile/translate_setup.hpp"
#include"Plugin/Setup.hpp"
#include"Util/Str.hpp"

namespace {

std::string translate_pubkey( Secp256k1::PubKey& pk
			    , char const valid_prefix[2]
			    , std::string const& key
			    , std::string const& value
			    ) {
	if (!Util::Str::ishex(value))
		return "Public key '" + key + "' must be a hexadecimal string.";
	if (value.length() != 66)
		return "Public key '" + key + "' must have 66 digits.";
	if (value[0] != valid_prefix[0] && value[0] != valid_prefix[1])
		return "Public key '" + key + "' must start with " + std::string(valid_prefix, 1) + ".";
	if (value[1] != '2' && value[1] != '3')
		return "Public key '" + key + "' must have Y sign digit of 2 or 3.";
	auto copy = value;
	copy[0] = '0';
	pk = Secp256k1::PubKey(copy);
	return "";
}

std::string translate_host_port( std::string& host
			       , int& port
			       , int default_port
			       , std::string const& key
			       , std::string const& value
			       ) {
	/* Accepted formats:
	 * [h:o:s:t]:port
	 * [h:o:s:t]
	 * host:port
	 * host
	 */
	if (value.size() == 0)
		return "Host specification '" + key + "' must be non-empty.";
	if (value[0] == '[') {
		auto hostend = std::find(value.begin() + 1, value.end(), ']');
		if (hostend == value.end())
			return "Host specification '" + key
			     + "' starts with [ but has no ending ]."
			     ;
		if (hostend + 1 == value.end()) {
			host = std::string(value.begin() + 1, hostend);
			port = default_port;
			return "";
		}
		if (*(hostend + 1) != ':')
			return "Host specification '" + key
			     + "' expecting : after [host]"
			     ;
		auto portstring = std::string(hostend + 2, value.end());
		if (portstring == "")
			return "Host specification '" + key
			     + "' expecting port after [host]:"
			     ;

		host = std::string(value.begin() + 1, hostend);
		port = std::stoi(portstring);
		return "";
	} else {
		auto hostend = std::find(value.begin(), value.end(), ':');
		if (hostend == value.end()) {
			host = value;
			port = default_port;
			return "";
		}
		if (hostend + 1 == value.end())
			return "Host specification '" + key
			     + "' expecting port after host:"
			     ;
		auto portstring = std::string(hostend + 1, value.end());
		host = std::string(value.begin(), hostend);
		port = std::stoi(portstring);
		return "";
	}
}

}

namespace Plugin { namespace OptFile {

std::string translate_setup( Plugin::Setup& setup
			   , std::map<std::string, std::string> const& opts
			   ) {
	auto it = opts.begin();
	auto err = std::string("");
	if ((it = opts.find("nid")) != opts.end())
		err = translate_pubkey(setup.node_id, "00", "nid", it->second);
	if (err != "")
		return err;
	if ((it = opts.find("nsig")) != opts.end()) {
		auto& value = it->second;
		if (!Util::Str::ishex(value))
			return "Signature 'nsig' "
			       "must be a hexadecimal string."
			       ;
		if (value.length() != 128)
			return "Signature 'nsig' "
			       "must have 128 digits."
			       ;
		setup.node_sig = Secp256k1::Signature(value);
	}

	if ((it = opts.find("cid")) != opts.end())
		err = translate_pubkey(setup.our_id, "Cc", "cid", it->second);
	if (err != "")
		return err;
	if ((it = opts.find("cpk")) != opts.end()) {
		auto& value = it->second;
		if (!Util::Str::ishex(value))
			return "Private key 'cpk' "
			       "must be a hexadecimal string."
			       ;
		if (value.length() != 64)
			return "Private key 'cpk' "
			       "must have 64 digits."
			       ;
		setup.our_priv_key = Secp256k1::PrivKey(value);
	}

	if ((it = opts.find("proxy")) != opts.end()) {
		setup.has_proxy = true;
		auto ret = translate_host_port( setup.proxy_host
					      , setup.proxy_port
					      , 9050
					      , "proxy"
					      , it->second
					      );
		if (ret != "")
			return ret;
	} else {
		setup.has_proxy = false;
	}

	/* TODO: multiserver.  */
	if ((it = opts.find("sid")) != opts.end()) {
		setup.servers.resize(1);
		err = translate_pubkey( setup.servers[0].id
				      , "55", "sid"
				      , it->second
				      );
	}
	if (err != "")
		return err;
	if ((it = opts.find("shost")) != opts.end()) {
		setup.servers.resize(1);
		err = translate_host_port( setup.servers[0].host
					 , setup.servers[0].port
					 , 29735
					 , "shost"
					 , it->second
					 );
	}
	if (err != "")
		return err;

	return "";
}

}}

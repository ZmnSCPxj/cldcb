#include<sstream>
#include"Secp256k1/PubKey.hpp"
#include"Server/AddRemove.hpp"
#include"Server/OptionsHandler.hpp"
#include"Server/TermLogger.hpp"
#include"Server/change_clients.hpp"

namespace {

std::string get_client_id(Secp256k1::PubKey const& pk) {
	auto os = std::ostringstream();
	os << pk;
	auto s = os.str();
	s[0] = 'c';
	return s;
}

}

namespace Server {

int
AddRemove::operator()(std::vector<std::string> params) {
	auto logger = Server::TermLogger();

	/* TODO: options, like the server directory.  */
	auto options = Server::OptionsHandler(logger);
	auto optret = options.handle(params);
	if (optret)
		return *optret;

	if (params.empty()) {
		logger.BROKEN("Nothing to add!");
		return 1;
	}

	auto new_clients = std::vector<Secp256k1::PubKey>();
	for (auto& param : params) {
		if (param.length() != 66) {
			logger.BROKEN( "Invalid client id: %s"
				     , param.c_str()
				     );
			return 1;
		}
		if (param[0] != 'c' && param[0] != 'C') {
			logger.BROKEN( "Invalid client id: %s"
				     , param.c_str()
				     );
			return 1;
		}

		auto c0 = param[0];
		param[0] = '0';

		auto cid = Secp256k1::PubKey();
		try {
			cid = Secp256k1::PubKey(param);
		} catch (Secp256k1::InvalidPubKey const&) {
			param[0] = c0;
			logger.BROKEN( "Invalid client id: %s"
				     , param.c_str()
				     );
			return 1;
		}
		new_clients.push_back(cid);
	}

	auto error = Server::change_clients( logger
					   , [ &new_clients
					     , &logger
					     , this
					     ](Server::ClientSet& clients) {
		for (auto& new_client : new_clients) {
			auto cid = get_client_id(new_client);
			if (clients.find(new_client) != clients.end()) {
				/* Exists in the clients file.  */
				if (mode == AddMode)
					logger.info( "%s already exists."
						   , cid.c_str()
						   );
				else {
					logger.debug( "%s being removed."
						    , cid.c_str()
						    );
					clients.erase(new_client);
				}
			} else {
				/* Does not exist in the client file.  */
				if (mode == AddMode) {
					logger.debug( "%s being added."
						    , cid.c_str()
						    );
					clients.insert(new_client);
				} else
					logger.info( "%s already nonexistent."
						   , cid.c_str()
						   );
			}
		}
	});

	if (error != "") {
		logger.BROKEN("Failed to update clients file: %s", error.c_str());
		return 1;
	}

	return 0;
}

}

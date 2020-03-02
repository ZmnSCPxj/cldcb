#include<sstream>
#include"Secp256k1/PubKey.hpp"
#include"Server/Add.hpp"
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
Add::operator()(std::vector<std::string> params) {
	auto logger = Server::TermLogger();

	/* TODO: options, like the server directory.  */

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
					     ](Server::ClientSet& clients) {
		for (auto& new_client : new_clients) {
			if (clients.find(new_client) != clients.end()) {
				logger.info( "%s already exists."
					   , get_client_id(new_client).c_str()
					   );
			} else {
				logger.debug( "%s being added."
					    , get_client_id(new_client).c_str()
					    );
				clients.insert(new_client);
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

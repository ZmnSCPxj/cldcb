#include<errno.h>
#include<sstream>
#include<string.h>
#include"Net/Connector.hpp"
#include"Net/SocketFd.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/PubKey.hpp"
#include"ServerTalker/Handshaker.hpp"
#include"ServerTalker/Messenger.hpp"
#include"ServerTalker/connect_then_handshake.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

std::string stringify_pubkey(char prefix, Secp256k1::PubKey const& pk) {
	auto os = std::ostringstream();
	os << pk;
	auto s = os.str();
	s[0] = prefix;
	return s;
}

}

namespace ServerTalker {

std::unique_ptr<ServerTalker::MessengerIf>
connect_then_handshake( Util::Logger& logger
		      , Net::Connector& connector
		      , Secp256k1::KeyPair const& client_id
		      , Secp256k1::PubKey const& server_id
		      , std::string const& server_host
		      , int server_port
		      ) {
	logger.debug( "ServerTalker: Connecting to [%1$s]:%2$d."
		    , server_host.c_str(), server_port
		    );

	auto sfd = connector.connect(server_host, server_port);
	if (!sfd) {
		auto my_errno = errno;
		logger.BROKEN( "ServerTalker: Failed to connect to "
			       "[%1$s]:%2$d: %3$s"
			     , server_host.c_str(), server_port
			     , strerror(my_errno)
			     );
		return nullptr;
	}
	logger.debug( "ServerTalker: Connected to [%1$s]:%2$d."
		    , server_host.c_str(), server_port
		    );

	/* Bound the lifetime of the Handshaker here.  */
	auto enc = ([ &sfd
		    , &logger
		    , &client_id
		    , &server_id
		    ]() {
		ServerTalker::Handshaker handshaker
			( logger
			, client_id
			, server_id
			, "CLDCB"
			, sfd
			);
		return handshaker.handshake();
	})();
	if (!enc) {
		auto my_errno = errno;
		logger.BROKEN( "ServerTalker: Failed handshake with "
			       "[%1$s]:%2$d: %s"
			     , server_host.c_str(), server_port
			     , strerror(my_errno)
			     );
		return nullptr;
	}
	logger.debug( "ServerTalker: Shook hands with %3$s@[%1$s]:%2$d "
		      "as %4$s."
		    , server_host.c_str(), server_port
		    , stringify_pubkey('5', server_id).c_str()
		    , stringify_pubkey('c', client_id.pub()).c_str()
		    );

	return Util::make_unique<ServerTalker::Messenger>
		( logger
		, std::move(sfd)
		, std::move(*enc)
		);
}

}

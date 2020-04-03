#ifndef CLDCB_SERVER_DAEMON_CLIENTLIST_HPP
#define CLDCB_SERVER_DAEMON_CLIENTLIST_HPP

#include<memory>
#include"Daemon/ClientAllow.hpp"

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace Daemon {

class ClientList : public Daemon::ClientAllow {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	explicit ClientList( Util::Logger& logger
			   , Daemon::Breaker& breaker
			   );

public:
	ClientList() =delete;
	~ClientList();

	/* Singleton constructor.  */
	static
	std::unique_ptr<ClientList> initialize( Util::Logger&
					      , Daemon::Breaker&
					      );

	/* Launch the SIGHUP monitor, which will reload the
	 * client_list on SIGHUP.
	 * This will immediately return with a 0.
	 */
	Ev::Io<int> launch();

	/* Determine if the specified client is in the list.  */
	bool has(Secp256k1::PubKey const&) const;
};

}

#endif /* CLDCB_SERVER_DAEMON_CLIENTLIST_HPP */

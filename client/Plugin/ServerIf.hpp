#ifndef CLDCB_CLIENT_PLUGIN_SERVERIF_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERIF_HPP

#include<cstdint>
#include<future>

namespace Plugin { class ServerIncrementIf; }

namespace Plugin {

/* Abstract interface to a backup server.
 *
 * The server interface is stateful; you should only call
 * new_update if the previous ServerIncrementIf
 * or ServerReuploadIf have been finished.
 */
class ServerIf {
public:
	virtual ~ServerIf() { }

	/* Tell the server the data version of an
	 * upcoming update.
	 * Then it returns an incremental-update
	 * interface to send the upcoming update
	 * on.
	 * Do *not* use this ServerIf until you
	 * have completely sent the incremental
	 * update.
	 * If the server is unable to back up, returns 
	 * nullptr.
	 */
	virtual
	std::future<std::unique_ptr<Plugin::ServerIncrementIf>>
	new_update( std::uint32_t data_version
		  ) =0;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERIF_HPP */

#ifndef CLDCB_CLIENT_PLUGIN_SERVERIF_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERIF_HPP

#include<cstdint>
#include<future>

namespace Plugin { class ServerResult; }

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
	 * If the data version is fine, it returns an
	 * increment result, yielding a ServerIncrementIf
	 * interface.
	 * If the server wants to get a reupload of the
	 * original file, returns a reupload result,
	 * yielding a ServerReuploadIf interface.
	 * If the server is unable to back up, returns 
	 * failure result.
	 */
	virtual
	std::future<Plugin::ServerResult>
	new_update( std::uint32_t data_version
		  ) =0;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERIF_HPP */

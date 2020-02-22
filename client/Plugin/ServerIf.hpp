#ifndef CLDCB_CLIENT_PLUGIN_SERVERIF_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERIF_HPP

#include<cstdint>
#include<future>
#include<vector>

namespace Plugin { class ServerResult; }

namespace Plugin {

/* Abstract interface to a backup server.
 */
class ServerIf {
public:
	virtual ~ServerIf() { }

	/* Send the specified incremental ciphertext to the
	 * server, with the specified data version.
	 * If the server is able to back it up, returns a
	 * successful result.
	 * If the server is unable to back up, returns a
	 * failure result.
	 * If the server wants to get a reupload of the
	 * original file, returns a reupload result.
	 */
	virtual
	std::future<Plugin::ServerResult>
	send( std::uint32_t data_version
	    , std::vector<std::uint8_t> ciphertext
	    ) =0;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERIF_HPP */

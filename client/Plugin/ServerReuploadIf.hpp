#ifndef CLDCB_CLIENT_PLUGIN_SERVERREUPLOADIF_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERREUPLOADIF_HPP

#include<cstdint>
#include<future>
#include<vector>

namespace Plugin {

/* Abstract interface to a backup server when the
 * server wants to get a copy of the database
 * prior to the most recent incremental update.
 */
class ServerReuploadIf {
public:
	virtual ~ServerReuploadIf() { }

	/* Send a chunk of the encrypted database.
	 * The chunk must be <= 65535 bytes.
	 * Returns true if the server backed it up
	 * okay, false otherwise.
	 * Precondition: completed() must not have
	 * been called yet.
	 * Precondition: any previous send_chunk()
	 * call has had its future already valid.
	 */
	virtual
	std::future<bool>
	send_chunk(std::vector<std::uint8_t> ciphertext) =0;

	/* Signal complete sending of the database.
	 * Precondition: any previous send_chunk()
	 * call has had its future already valid.
	 */
	virtual
	std::future<void> completed() =0;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERREUPLOADIF_HPP */

#ifndef CLDCB_CLIENT_PLUGIN_SERVERINCREMENTIF_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERINCREMENTIF_HPP

#include<cstdint>
#include<future>
#include<vector>

namespace Plugin {

class ServerIncrementIf {
public:
	virtual ~ServerIncrementIf() { }

	/* Give a chunk of data for the current incremental
	 * update to the server.
	 * The chunk must be <= 65000 bytes.
	 * Returns true if the server backed it up okay,
	 * false otherwise.
	 * Precondition: increment_completed() must not have been
	 * called yet.
	 * Precondition: any previous send_increment_chunk() call
	 * has had its future already valid.
	 */
	virtual
	std::future<bool>
	send_increment_chunk(std::vector<std::uint8_t> ciphertext) =0;

	/* Signal complete sending of the current increment.
	 * Returns true if the server backed it up okay,
	 * false otherwise.
	 * Precondition: any previous send_increment_chunk() call
	 * has had its future already valid.
	 * Postcondition: this object can no longer be used after
	 * this call is invoked.
	 */
	virtual
	std::future<bool> increment_completed() =0;
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERINCREMENTIF_HPP */

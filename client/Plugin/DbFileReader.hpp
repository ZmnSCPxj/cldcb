#ifndef CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP
#define CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Plugin {

class DbFileReader {
public:
	virtual ~DbFileReader() { }

	/* Ab instance of reading the DB file from the beginning.  */
	class Session {
	public:
		virtual ~Session() { }

		/* Returns a 0-length vector at EOF.
		 * Returned vector is max_length or less.
		 * Consecutive calls read consecutive
		 * sections of the file.
		 */
		virtual
		std::vector<std::uint8_t> read(unsigned int max_length) =0;
	};

	/* Starts a new session of reading the DB file.  */
	virtual 
	std::unique_ptr<Session> start() =0;

};

}

#endif /* CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP */

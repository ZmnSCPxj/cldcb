#ifndef CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP
#define CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP

#include<cstdint>
#include<fstream>
#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace Plugin {

class DbFileReader {
private:
	std::string path;
public:
	DbFileReader() =delete;
	explicit DbFileReader(std::string path_) : path(std::move(path_)) { }

	/* Ab instance of reading the DB file from the beginning.  */
	class Session {
	private:
		std::ifstream file;

		Session() =delete;
		Session(Session const&) =delete;
		Session(Session&&) =delete;

		Session(std::ifstream file_) : file(std::move(file_)) { }

	public:
		/* Returns a 0-length vector at EOF.
		 * Returned vector is max_length or less.
		 * Consecutive calls read consecutive
		 * sections of the file.
		 */
		std::vector<std::uint8_t> read(unsigned int max_length);

		friend class Plugin::DbFileReader;
	};

	/* Starts a new session of reading the DB file.  */
	std::unique_ptr<Session> start();

};

}

#endif /* CDLCB_CLIENT_PLUGIN_DBFILEREADER_HPP */

#ifndef CLDCB_SERVER_ARCHIVE_INCREMENTALWRITER_HPP
#define CLDCB_SERVER_ARCHIVE_INCREMENTALWRITER_HPP

#include<cstdint>
#include<vector>
#include"Net/Fd.hpp"

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Util { class Logger; }

namespace Archive {

/* An object that writes a single incremental update to the
 * given fd.
 * On construction, we sample the current size of the file,
 * and if this object is destructed without incremental_end
 * having completed, it truncates the file back to the
 * initial size.
 * This object can be integrated into an object that fully
 * implements Backup::IncrementalStorage.
 */
class IncrementalWriter {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	std::uint64_t last_length;
	std::uint32_t data_version;
	std::uint16_t count;
	Net::Fd fd;

	std::uint64_t orig_size;

public:
	explicit
	IncrementalWriter( Util::Logger& logger
			 , Ev::ThreadPool& threadpool
			 , std::uint32_t data_version
			 , std::uint16_t count
			 , Net::Fd fd
			 );
	~IncrementalWriter();

	Ev::Io<bool>
	incremental_chunk(std::vector<std::uint8_t> chunk);
	Ev::Io<bool>
	incremental_end();
/* Executed in th backgroud in the thread pool, to prevent blocking
 * of the main thread.
 */
private:
	bool increment_chunk_back(std::vector<std::uint8_t> chunk);
	bool increment_end_back();

};

}

#endif /* CLDCB_SERVER_ARCHIVE_INCREMENTALWRITER_HPP */

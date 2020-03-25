#ifndef CLDCB_SERVER_ARCHIVE_READER_HPP
#define CLDCB_SERVER_ARCHIVE_READER_HPP

#include<cstdint>
#include<utility>
#include"Backup/DataReader.hpp"
#include"Net/Fd.hpp"

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Util { class Logger; }

namespace Archive {

class Reader : public Backup::DataReader {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;

	Net::Fd fd;

	bool reupload_done;
	bool incremental_started;

public:
	Reader(Reader&&) =default;

	explicit
	Reader( Util::Logger& logger_
	      , Ev::ThreadPool& threadpool_
	      , Net::Fd fd_
	      ) : logger(logger_)
		, threadpool(threadpool_)
		, fd(std::move(fd_))
		, reupload_done(false)
		, incremental_started(false)
		{ }

	Ev::Io<std::unique_ptr<std::vector<std::uint8_t>>>
	backedup_reupload_chunk() override;
	Ev::Io<std::unique_ptr<Backup::DataReader::IncrementMsg>>
	backedup_incremental_get() override;

private:
	/* Functions executed in background threadpool.  */
	std::unique_ptr<std::vector<std::uint8_t>>
	backedup_reupload_chunk_back();
	std::unique_ptr<Backup::DataReader::IncrementMsg>
	backedup_incremental_get_back();

	/* Return nullptr on failure, data_version if ok.  */
	std::unique_ptr<std::uint32_t> read_data_version();
	/* Return nullptr on failure, chunk size if ok.  */
	std::unique_ptr<std::uint16_t> read_size();
	/* Return false on failure, true if slurped footer OK.  */
	bool skip_footer();
	/* Return nullptr on failure, chunk if ok.  */
	std::unique_ptr<std::vector<std::uint8_t>>
	read_chunk(std::size_t);
};

}

#endif /* CLDCB_SERVER_ARCHIVE_READER_HPP */

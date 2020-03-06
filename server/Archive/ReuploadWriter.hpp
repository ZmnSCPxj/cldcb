#ifndef CLDCB_SERVER_ARCHIVE_REUPLOADWRITER_HPP
#define CLDCB_SERVER_ARCHIVE_REUPLOADWRITER_HPP

#include<cstdint>
#include<functional>
#include<utility>
#include<vector>
#include"Net/Fd.hpp"

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Util { class Logger; }

namespace Archive {

class ReuploadWriter {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	Net::Fd fd;
	std::function<Ev::Io<bool>(Net::Fd)> continuation;

public:
	ReuploadWriter( Util::Logger& logger_
		      , Ev::ThreadPool& threadpool_
		      , Net::Fd fd_
		      , std::function<Ev::Io<bool>(Net::Fd)> continuation_
		      ) : logger(logger_)
			, threadpool(threadpool_)
			, fd(std::move(fd_))
			, continuation(std::move(continuation_))
			{ }

	Ev::Io<bool> reupload_chunk(std::vector<std::uint8_t> chunk);
	Ev::Io<bool> reupload_end();
private:
	bool reupload_chunk_back(std::vector<std::uint8_t> chunk);
	bool reupload_end_back();
};

}

#endif /* CLDCB_SERVER_ARCHIVE_REUPLOADWRITER_HPP */

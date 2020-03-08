#ifndef CLDCB_SERVER_ARCHIVE_INCREMENTALAPPEND_HPP
#define CLDCB_SERVER_ARCHIVE_INCREMENTALAPPEND_HPP

#include<string>

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Net { class Fd; }
namespace Util { class Logger; }

namespace Archive {

/* A sequence where we append the contents of a specific
 * file to a given file descriptor.
 */
class IncrementalAppend {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	std::string incremental_filename;

public:
	IncrementalAppend( Util::Logger& logger_
			 , Ev::ThreadPool& threadpool_
			 , std::string incremental_filename_
			 ) : logger(logger_)
			   , threadpool(threadpool_)
			   , incremental_filename(std::move(incremental_filename_))
			   { }

	/* Append to the given file descriptor.  */
	Ev::Io<bool> append_to(Net::Fd);
private:
	bool append_to_back(Net::Fd);
};

}

#endif /* CLDCB_SERVER_ARCHIVE_INCREMENTALAPPEND_HPP */

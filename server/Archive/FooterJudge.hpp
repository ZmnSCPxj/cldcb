#ifndef CLDCB_SERVER_ARCHIVE_FOOTERJUDGE_HPP
#define CLDCB_SERVER_ARCHIVE_FOOTERJUDGE_HPP

#include<cstdint>
#include<string>
#include"Net/Fd.hpp"

namespace Ev { template<typename a> class Io; }
namespace Ev { class ThreadPool; }
namespace Util { class Logger; }

namespace Archive {


/* Determines if the given file (which must be seekable)
 * has a footer, and whether the given `data_version`
 * matches or not.
 */
class FooterJudge {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	std::uint16_t max_count;
public:
	enum JudgmentType
	{ Append
	, TruncateThenAppend
	, Reupload
	};
	struct Judgment {
		JudgmentType type;
		/* If Append or TruncateThenAppend, the
		 * opened file.
		 * The file is opened O_RDWR | O_APPEND.
		 */
		Net::Fd fd;
		/* If TruncateThenAppend, the number of
		 * bytes to remove.
		 */
		std::size_t to_remove;
	};

	FooterJudge() =delete;
	FooterJudge(FooterJudge const&) =default;
	FooterJudge(FooterJudge&&) =default;
	FooterJudge& operator=(FooterJudge const&) =default;
	FooterJudge& operator=(FooterJudge&&) =default;

	FooterJudge( Util::Logger& logger_
		   , Ev::ThreadPool& threadpool_
		   , std::uint16_t max_count_
		   ) : logger(logger_)
		     , threadpool(threadpool_)
		     , max_count(max_count_)
		     { }

	/* Open the file, check the footer about what to do
	 * given the current data_version.
	 */
	Ev::Io<Judgment> judge(std::string filename, std::uint32_t data_version);
private:
	Judgment judge_back(std::string filename, std::uint32_t data_version);
};

}

#endif /* CLDCB_SERVER_ARCHIVE_FOOTERJUDGE_HPP */

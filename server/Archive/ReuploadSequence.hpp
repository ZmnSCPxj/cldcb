#ifndef CLDCB_ARCHIVE_REUPLOADSEQUENCE_HPP
#define CLDCB_ARCHIVE_REUPLOADSEQUENCE_HPP

#include<memory>
#include"Archive/IncrementalAppend.hpp"
#include"Archive/ReuploadWriter.hpp"
#include"Archive/Unlinker.hpp"
#include"Backup/ReuploadStorage.hpp"

namespace Archive {

/* This sequence is performed after an incremental sequence.
 * It creates a temporary file, the temp_reupload_filename,
 * to write the reupload data to.
 * On completion of the reupload data, it appends the given
 * incremental file, then renames the temporary file to the
 * actual archive file.
 */
class ReuploadSequence : public Backup::ReuploadStorage {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	Archive::IncrementalAppend appender;
	Archive::Unlinker temp_incremental_unlinker;
	std::unique_ptr<Archive::ReuploadWriter> writer;
	std::unique_ptr<Archive::Unlinker> unlinker;
	std::string temp_reupload_filename;
	std::string archive_filename;

public:
	ReuploadSequence() =delete;

	ReuploadSequence( Util::Logger& logger_
			, Ev::ThreadPool& threadpool_
			, std::string temp_incremental_filename_
			, Archive::Unlinker temp_incremental_unlinker_
			, std::string temp_reupload_filename_
			, std::string archive_filename_
			) : logger(logger_)
			  , threadpool(threadpool_)
			  , appender( logger, threadpool
				    , std::move(temp_incremental_filename_)
				    )
			  , temp_incremental_unlinker(
				std::move(temp_incremental_unlinker_)
			    )
			  , writer()
			  , unlinker()
			  , temp_reupload_filename(std::move(temp_reupload_filename_))
			  , archive_filename(archive_filename_)
			  { }

	Ev::Io<bool> reupload_chunk(std::vector<std::uint8_t> chunk) override;
	Ev::Io<bool> reupload_end() override;

private:
	Ev::Io<bool> create_writer();
	Ev::Io<bool> move_over_archive();
};

}

#endif /* CLDCB_ARCHIVE_REUPLOADSEQUENCE_HPP */

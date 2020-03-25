#ifndef CLDCB_SERVER_ARCHIVE_INCREMENTALTHENREUPLOADSEQUENCE_HPP
#define CLDCB_SERVER_ARCHIVE_INCREMENTALTHENREUPLOADSEQUENCE_HPP

#include<memory>
#include"Archive/IncrementalWriter.hpp"
#include"Archive/Unlinker.hpp"
#include"Backup/IncrementalStorage.hpp"

namespace Archive {

class IncrementalThenReuploadSequence
		: public Backup::IncrementalStorage {
private:
	Util::Logger& logger;
	Ev::ThreadPool& threadpool;
	std::unique_ptr<Archive::IncrementalWriter> writer;
	std::unique_ptr<Archive::Unlinker> unlinker;
	std::string temp_incremental_filename;
	std::string temp_reupload_filename;
	std::string archive_filename;
	std::uint32_t data_version;

public:
	IncrementalThenReuploadSequence() =delete;
	IncrementalThenReuploadSequence
		(IncrementalThenReuploadSequence&&)=default;

	IncrementalThenReuploadSequence( Util::Logger& logger_
				       , Ev::ThreadPool& threadpool_
				       , std::string temp_incremental_filename_
				       , std::string temp_reupload_filename_
				       , std::string archive_filename_
				       , std::uint32_t data_version_
				       ) : logger(logger_)
					 , threadpool(threadpool_)
					 , writer()
					 , unlinker()
					 , temp_incremental_filename(std::move(temp_incremental_filename_))
					 , temp_reupload_filename(std::move(temp_reupload_filename_))
					 , archive_filename(std::move(archive_filename_))
					 , data_version(data_version_)
					 { }

	bool will_response_reupload() const override { return true; }
	Ev::Io<bool>
	incremental_chunk(std::vector<std::uint8_t> chunk) override;
	Ev::Io<bool> incremental_end() override;
	Ev::Io<std::unique_ptr<Backup::ReuploadStorage>>
	get_response_storage() override;

private:
	Ev::Io<bool> create_writer();
};

}

#endif /* CLDCB_SERVER_ARCHIVE_INCREMENTALTHENREUPLOADSEQUENCE_HPP */

#ifndef CLDCB_SERVER_ARCHIVE_INCREMENTALONLYSEQUENCE_HPP
#define CLDCB_SERVER_ARCHIVE_INCREMENTALONLYSEQUENCE_HPP

#include<utility>
#include"Ev/Io.hpp"
#include"Archive/IncrementalWriter.hpp"
#include"Backup/IncrementalStorage.hpp"
#include"Backup/ReuploadStorage.hpp"

namespace Archive {

/* A Backup::IncrementalStorage instance, where
 * the server has determined already that the
 * only thing that needs to be done is to append
 * the increment update to the archive.
 */
class IncrementalOnlySequence : public Backup::IncrementalStorage {
private:
	Archive::IncrementalWriter writer;

public:
	IncrementalOnlySequence( Util::Logger& logger
			       , Ev::ThreadPool& threadpool
			       , std::uint32_t data_version
			       , std::uint16_t count
			       , Net::Fd fd
			       ) : writer( logger
					 , threadpool
					 , data_version
					 , count
					 , std::move(fd)
					 )
				 { }

	bool will_response_reupload() const override {
		return false;
	}
	Ev::Io<bool>
	incremental_chunk(std::vector<std::uint8_t> chunk) override {
		return writer.incremental_chunk(std::move(chunk));
	}
	Ev::Io<bool> incremental_end() override {
		return writer.incremental_end();
	}
	Ev::Io<std::unique_ptr<Backup::ReuploadStorage>>
	get_response_storage() override {
		return Ev::lift_io(std::unique_ptr<Backup::ReuploadStorage>());
	}
};

}

#endif /* CLDCB_SERVER_ARCHIVE_INCREMENTALONLYSEQUENCE_HPP */

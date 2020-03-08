#include<assert.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/IncrementalWriter.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"S.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include"Util/Logger.hpp"
#include"Util/Rw.hpp"

namespace Archive {

IncrementalWriter::IncrementalWriter( Util::Logger& logger_
				    , Ev::ThreadPool& threadpool_
				    , std::uint32_t data_version_
				    , std::uint16_t count_
				    , Net::Fd fd_
				    ) : logger(logger_)
				      , threadpool(threadpool_)
				      , last_length(0)
				      , data_version(data_version_)
				      , count(count_)
				      , fd(std::move(fd_))
				      , orig_size()
				      {
	if (!fd)
		return;
	auto size = lseek(fd.get(), off_t(0), SEEK_END);
	if (size < off_t(0)) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::IncrementalWriter: "
			       "closing, cannot get file size of <fd %d>: %s"
			     , fd.get()
			     , strerror(my_errno)
			     );
		/* Enter an invalid state. */
		fd.reset();
	} else {
		orig_size = std::uint64_t(size);
	}
}

IncrementalWriter::~IncrementalWriter() {
	if (!fd)
		return;
	logger.unusual( "Rolling back incomplete update <fd %d>"
		      , fd.get()
		      );
	auto res = ftruncate(fd.get(), off_t(orig_size));
}

Ev::Io<bool>
IncrementalWriter::incremental_chunk(std::vector<std::uint8_t> chunk) {
	assert(chunk.size() != 0);
	assert(chunk.size() <= 65535);
	/* In principle we could cache a few chunks in-memory, then
	 * send multiple chunks at once to the threadpool, reducing
	 * threadpool overhead.
	 */
	if (!fd) {
		logger.unusual("Archive::IncrementalWriter: Closed fd.");
		return Ev::lift_io(false);
	}
	auto pchunk = std::make_shared<std::vector<std::uint8_t>>(
		std::move(chunk)
	);
	return threadpool.background<bool>([this, pchunk]() {
		return increment_chunk_back(std::move(*pchunk));
	});
}
Ev::Io<bool>
IncrementalWriter::incremental_end() {
	if (!fd) {
		logger.unusual("Archive::IncrementalWriter: Closed fd.");
		return Ev::lift_io(false);
	}
	return threadpool.background<bool>([this]() {
		return increment_end_back();
	});
}

bool
IncrementalWriter::increment_chunk_back(std::vector<std::uint8_t> chunk) {
	/* Write the size.  */
	auto size = std::uint16_t(chunk.size());
	auto sizebuff = std::vector<std::uint8_t>();
	S::serialize(sizebuff, size);

	auto sres = Util::Rw::write_all( fd.get()
				       , &sizebuff[0], sizebuff.size()
				       );
	if (!sres) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to write chunk size to <fd %d>: %s"
			     , fd.get(), strerror(my_errno)
			     );
		return false;
	}
	last_length += sizebuff.size();

	/* Write the actual chunk.  */
	auto cres = Util::Rw::write_all( fd.get()
				       , &chunk[0], chunk.size()
				       );
	if (!cres) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to write chunk data to <fd %d>: %s"
			     , fd.get(), strerror(my_errno)
			     );
		return false;
	}
	last_length += chunk.size();

	return true;
}

bool
IncrementalWriter::increment_end_back() {
	/* Write the terminating 0-size chunk.  */
	auto size = std::uint16_t(0);
	auto sizebuff = std::vector<std::uint8_t>();
	S::serialize(sizebuff, size);

	auto sres = Util::Rw::write_all( fd.get()
				       , &sizebuff[0], sizebuff.size()
				       );
	if (!sres) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to write null chunk to <fd %d>: %s"
			     , fd.get(), strerror(my_errno)
			     );
		return false;
	}
	last_length += sizebuff.size();

	/* Serialize the footer.  */
	auto footer = std::vector<std::uint8_t>();
	S::serialize(footer, last_length);
	S::serialize(footer, data_version);
	S::serialize(footer, count);
	auto checksum = Sha256::fun(&footer[0], footer.size());
	S::serialize(footer, checksum);
	/* Write the footer.  */
	auto fres = Util::Rw::write_all( fd.get()
				       , &footer[0], footer.size()
				       );
	if (!fres) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to write footer to <fd %d>: %s"
			     , fd.get(), strerror(my_errno)
			     );
		return false;
	}

	/* SYNC!  */
	auto syncres = fsync(fd.get());
	if (syncres < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to sync <fd %d>: %s"
			     , fd.get(), strerror(my_errno)
			     );
		return false;
	}

	/* Close.  */
	logger.debug( "Closing <fd %d>." , fd.get());
	fd.reset();

	return true;
}

}

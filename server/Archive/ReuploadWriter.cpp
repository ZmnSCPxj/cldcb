#include<assert.h>
#include<errno.h>
#include<stdexcept>
#include<string.h>
#include"Archive/ReuploadWriter.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"S.hpp"
#include"Util/Logger.hpp"
#include"Util/Rw.hpp"

namespace Archive {

Ev::Io<bool>
ReuploadWriter::reupload_chunk(std::vector<std::uint8_t> chunk) {
	assert(chunk.size() != 0);
	assert(chunk.size() <= 65535);
	if (!fd) {
		logger.unusual("Archive::ReuploadWriter: closed fd.");
		return Ev::lift_io(false);
	}
	auto pchunk = std::make_shared<std::vector<std::uint8_t>>(
		std::move(chunk)
	);
	return threadpool.background<bool>([this, pchunk]() {
		return reupload_chunk_back(std::move(*pchunk));
	});
}
bool
ReuploadWriter::reupload_chunk_back(std::vector<std::uint8_t> chunk) {
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

	return true;
}
Ev::Io<bool>
ReuploadWriter::reupload_end() {
	if (!fd) {
		logger.unusual("Archive::ReuploadWriter: Closed fd.");
		return Ev::lift_io(false);
	}
	return threadpool.background<bool>([this]() {
		return reupload_end_back();
	}).then<bool>([this](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		/* Hand over control of the fd.  */
		return continuation(std::move(fd));
	});
}
bool
ReuploadWriter::reupload_end_back() {
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

	return true;
}

}

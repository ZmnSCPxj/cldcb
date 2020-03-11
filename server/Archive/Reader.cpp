#include<string.h>
#include"Archive/Reader.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Util/Logger.hpp"
#include"Util/Rw.hpp"
#include"Util/make_unique.hpp"

namespace Archive {

Ev::Io<std::unique_ptr<std::vector<std::uint8_t>>>
Reader::backedup_reupload_chunk() {
	return threadpool.background<std::unique_ptr<std::vector<std::uint8_t>>>
		([this]() {
		return backedup_reupload_chunk_back();
	});
}
Ev::Io<std::unique_ptr<Backup::DataReader::IncrementMsg>>
Reader::backedup_incremental_get() {
	return threadpool.background<std::unique_ptr<Backup::DataReader::IncrementMsg>>([this]() {
		return backedup_incremental_get_back();
	});
}
std::unique_ptr<std::vector<std::uint8_t>>
Reader::backedup_reupload_chunk_back() {
	if (reupload_done) {
		logger.BROKEN( "Archive::Reader: reupload already done "
			       "on <fd %d>, "
			       "but we were asked to get more? BUG."
			     , fd.get()
			     );
		return nullptr;
	}

	auto size = read_size();
	if (!size)
		return nullptr;

	if (*size == 0) {
		/* End of reupload section.  */
		logger.debug( "Archive::Reader: completed reupload section "
			      "on <fd %d>"
			    , fd.get()
			    );

		reupload_done = true;
		incremental_started = false;

		/* Return empty buffer.  */
		return Util::make_unique<std::vector<std::uint8_t>>();
	}

	return read_chunk(std::size_t(*size));
}

std::unique_ptr<Backup::DataReader::IncrementMsg>
Reader::backedup_incremental_get_back() {
	typedef Backup::DataReader::IncrementMsg RetT;

	if (!reupload_done) {
		logger.BROKEN( "Archive::Reader: reupload not yet done on "
			       "<fd %d>, but already asked for incremental "
			       "data? BUG."
			     , fd.get()
			     );
		return nullptr;
	}

	if (!incremental_started) {
		auto data_version = read_data_version();
		if (!data_version)
			return nullptr;
		incremental_started = true;
		return Util::make_unique<RetT>( Backup::DataReader::New
					      , *data_version
					      , std::vector<std::uint8_t>()
					      );
	}

	auto size = read_size();
	if (!size)
		return nullptr;

	if (*size != 0) {
		auto chunk = read_chunk(*size);
		if (!chunk)
			return nullptr;
		return Util::make_unique<RetT>( Backup::DataReader::Chunk
					      , 0
					      , std::move(*chunk)
					      );
	}

	/* Either New or EndAll.  */
	auto fres = skip_footer();
	if (!fres)
		return nullptr;

	auto data_version = read_data_version();
	if (!data_version)
		/* End it all.  */
		return Util::make_unique<RetT>( Backup::DataReader::EndAll
					      , 0
					      , std::vector<std::uint8_t>()
					      );
	else
		return Util::make_unique<RetT>( Backup::DataReader::New
					      , *data_version
					      , std::vector<std::uint8_t>()
					      );
}

std::unique_ptr<std::uint32_t>
Reader::read_data_version() {
	std::uint8_t buffer[4];
	auto rsize = sizeof(buffer);
	auto rres = Util::Rw::read_all(fd.get(), buffer, rsize);
	if (!rres) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::Reader: Failed to read data_version "
			       "from <fd %d>: %s"
			     , fd.get()
			     , strerror(my_errno)
			     );
		return nullptr;
	}
	if (rsize == 0) {
		/* Could just be no more updates... */
		logger.debug( "Archive::Reader: end-of-file <fd %d>."
			    , fd.get()
			    );
		return nullptr;
	}
	if (rsize < sizeof(buffer)) {
		logger.BROKEN( "Archive::Reader: incomplete data_version "
			       "from <fd %d>."
			     , fd.get()
			     );
		return nullptr;
	}

	auto data_version = (std::uint32_t(buffer[0]) << 24)
			  + (std::uint32_t(buffer[1]) << 16)
			  + (std::uint32_t(buffer[2]) << 8)
			  + (std::uint32_t(buffer[3]) << 0)
			  ;
	return Util::make_unique<std::uint32_t>(data_version);
}
std::unique_ptr<std::uint16_t>
Reader::read_size() {
	std::uint8_t buffer[2];
	auto rsize = sizeof(buffer);
	auto rres = Util::Rw::read_all(fd.get(), buffer, rsize);
	if (!rres) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::Reader: Failed to read chunk size "
			       "from <fd %d>: %s"
			     , fd.get()
			     , strerror(my_errno)
			     );
		return nullptr;
	}
	if (rsize < sizeof(buffer)) {
		logger.BROKEN( "Archive::Reader: incomplete chunk size "
			       "from <fd %d>."
			     , fd.get()
			     );
		return nullptr;
	}

	auto chunk_size = (std::uint16_t(buffer[0]) << 8)
			+ (std::uint16_t(buffer[1]) << 0)
			;
	return Util::make_unique<std::uint16_t>(chunk_size);
}
bool
Reader::skip_footer() {
	auto static constexpr footer_size = std::size_t( 8
						       + 4
						       + 2
						       + 32
						       );
	std::uint8_t buffer[footer_size];
	auto rsize = sizeof(buffer);
	auto rres = Util::Rw::read_all(fd.get(), buffer, rsize);
	if (!rres) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::Reader: Failed to read footer "
			       "from <fd %d>: %s"
			     , fd.get()
			     , strerror(my_errno)
			     );
		return false;
	}
	if (rsize < sizeof(buffer)) {
		logger.BROKEN( "Archive::Reader: incomplete footer "
			       "from <fd %d>."
			     , fd.get()
			     );
		return false;
	}
	return true;
}

std::unique_ptr<std::vector<std::uint8_t>>
Reader::read_chunk(std::size_t size) {
	auto ret = std::vector<std::uint8_t>(size);
	auto rsize = ret.size();
	auto rres = Util::Rw::read_all(fd.get(), &ret[0], rsize);
	if (!rres) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::Reader: Failed to read chunk "
			       "from <fd %d>: %s"
			     , fd.get()
			     , strerror(my_errno)
			     );
		return nullptr;
	}
	if (rsize != ret.size()) {
		logger.BROKEN( "Archive::Reader: Unexpected EOF while "
			       "reading chunk from <fd %d>."
			     , fd.get()
			     );
		return nullptr;
	}
	return Util::make_unique<std::vector<std::uint8_t>>(std::move(ret));
}

}

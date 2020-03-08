#include<assert.h>
#include<fcntl.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include"Archive/IncrementalThenReuploadSequence.hpp"
#include"Archive/ReuploadSequence.hpp"
#include"Ev/Io.hpp"
#include"Net/Fd.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Archive {

Ev::Io<bool>
IncrementalThenReuploadSequence::create_writer() {
	assert(!unlinker);
	auto fd = Net::Fd();
	do {
		fd = Net::Fd(::open( temp_incremental_filename.c_str()
				   , O_TRUNC 
				   | O_WRONLY
				   | O_CREAT
				   , 0400
				   ));
	} while (!fd && errno == EINTR);
	if (!fd) {
		auto my_errno = errno;
		logger.BROKEN( "Could not create temporary "
			       "incremental storage %s: %s"
			     , temp_incremental_filename.c_str()
			     , strerror(my_errno)
			     );
		return Ev::lift_io(false);
	}
	logger.debug( "Opened temporary incremental storage %s "
		      "for writing as <fd %d>."
		    , temp_incremental_filename.c_str()
		    , fd.get()
		    );
	unlinker = Util::make_unique<Archive::Unlinker>
		/* Do not std::move: we need this later to copy the
		 * incremental data to the reupload file.
		 */
		( temp_incremental_filename
		);
	writer = Util::make_unique<Archive::IncrementalWriter>
		( logger
		, threadpool
		, data_version
		, 0 /* count */
		, std::move(fd)
		);

	return Ev::lift_io(true);
}

Ev::Io<bool>
IncrementalThenReuploadSequence::incremental_chunk(std::vector<std::uint8_t> chunk) {
	auto pchunk = std::make_shared<std::vector<std::uint8_t>>(
		std::move(chunk)
	);
	return Ev::lift_io(0).then<bool>([this](int) {
		if (!writer)
			return create_writer();
		return Ev::lift_io(true);
	}).then<bool>([this, pchunk](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		return writer->incremental_chunk(std::move(*pchunk));
	});
}

Ev::Io<bool>
IncrementalThenReuploadSequence::incremental_end() {
	return Ev::lift_io(0).then<bool>([this](int) {
		if (!writer)
			return create_writer();
		return Ev::lift_io(true);
	}).then<bool>([this](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		return writer->incremental_end();
	});
}

Ev::Io<std::unique_ptr<Backup::ReuploadStorage>>
IncrementalThenReuploadSequence::get_response_storage() {
	typedef std::unique_ptr<Backup::ReuploadStorage> RetT;
	assert(writer);
	assert(unlinker);
	return Ev::lift_io(0).then<RetT>([this](int) {
		auto ret = Util::make_unique<Archive::ReuploadSequence>
			( logger
			, threadpool
			, std::move(temp_incremental_filename)
			, std::move(*unlinker)
			, std::move(temp_reupload_filename)
			, std::move(archive_filename)
			);
		unlinker = nullptr;
		writer = nullptr;
		return Ev::lift_io<RetT>(std::move(ret));
	});
}

}

#include<assert.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/ReuploadSequence.hpp"
#include"Ev/Io.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Archive {

Ev::Io<bool>
ReuploadSequence::create_writer() {
	/* TODO: we should really do this opening in the
	 * threadpool.
	 */
	assert(!unlinker);
	auto fd = Net::Fd();
	do {
		fd = Net::Fd(::open( temp_reupload_filename.c_str()
				   , O_TRUNC 
				   | O_WRONLY
				   | O_CREAT
				   , 0600
				   ));
	} while (!fd && errno == EINTR);

	if (!fd) {
		auto my_errno = errno;
		logger.BROKEN( "Could not create temporary "
			       "archive %s: %s"
			     , temp_reupload_filename.c_str()
			     , strerror(my_errno)
			     );
		return Ev::lift_io(false);
	}
	logger.debug( "Opened temporary archive %s "
		      "for writing as <fd %d>."
		    , temp_reupload_filename.c_str()
		    , fd.get()
		    );

	unlinker = Util::make_unique<Archive::Unlinker>
		/* Do not std::move: we need this later to rename.  */
		( temp_reupload_filename
		);
	writer = Util::make_unique<Archive::ReuploadWriter>
		( logger
		, threadpool
		, std::move(fd)
		, [this](Net::Fd fd) {
			return appender.append_to(std::move(fd));
		}
		);

	return Ev::lift_io(true);
}

Ev::Io<bool>
ReuploadSequence::reupload_chunk(std::vector<std::uint8_t> chunk) {
	auto pchunk = std::make_shared<std::vector<std::uint8_t>>(
		std::move(chunk)
	);
	return Ev::lift_io(0).then<bool>([this](int) {
		if (!writer) {
			return create_writer();
		} else
			return Ev::lift_io(true);
	}).then<bool>([this, pchunk](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		return writer->reupload_chunk(std::move(*pchunk));
	});
}

Ev::Io<bool>
ReuploadSequence::reupload_end() {
	return Ev::lift_io(0).then<bool>([this](int) {
		if (!writer) {
			return create_writer();
		}
		return Ev::lift_io(true);
	}).then<bool>([this](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		return writer->reupload_end();
	}).then<bool>([this](bool ok) {
		if (!ok)
			return Ev::lift_io(false);
		return move_over_archive();
	});
}

Ev::Io<bool>
ReuploadSequence::move_over_archive() {
	auto res = rename( temp_reupload_filename.c_str()
			 , archive_filename.c_str()
			 );
	/* FIXME: `rename` may fail even after successfully performing
	 * the rename!
	 * We should save the inode of the temp filename and compare
	 * to the archive filename, ignoring the result of rename().
	 * Alternately we can link() the temp file over the archive
	 * file and then release it.
	 */
	if (res < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Archive::ReuploadSequence: failed to rename "
			       "%s to %s: %s"
			     , temp_reupload_filename.c_str()
			     , archive_filename.c_str()
			     , strerror(my_errno)
			     );
		return Ev::lift_io(false);
	}
	/* kick the unlinker: we alrady successfully renamed what it is
	 * guarding.
	 * But let the incremental unlinker still unlink afterwards.
	 */
	unlinker->do_not_unlink();
	return Ev::lift_io(true);
}

}


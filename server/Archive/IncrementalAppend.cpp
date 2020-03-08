#include<cstdint>
#include<errno.h>
#include<fcntl.h>
#include<memory>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/IncrementalAppend.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Net/Fd.hpp"
#include"Util/Logger.hpp"
#include"Util/Rw.hpp"

namespace Archive {

Ev::Io<bool>
IncrementalAppend::append_to(Net::Fd dst_fd) {
	auto p_dst_fd = std::make_shared<Net::Fd>(std::move(dst_fd));
	return threadpool.background<bool>([this, p_dst_fd]() {
		return append_to_back(std::move(*p_dst_fd));
	});
}
bool
IncrementalAppend::append_to_back(Net::Fd dst_fd) {
	if (!dst_fd) {
		logger.unusual( "Archive::IncrementalAppend: "
				"Destination fd already closed."
			      );
		return false;
	}

	auto seekres = lseek(dst_fd.get(), off_t(0), SEEK_END);
	if (seekres < off_t(0)) {
		auto my_errno = errno;
		logger.unusual( "Arhive::IncrementalAppend: "
				"Could not seek to end of file "
				"of <fd %d>: %s"
			      , dst_fd.get()
			      , strerror(my_errno)
			      );
		return false;
	}

	auto src_fd = Net::Fd();
	do {
		src_fd = Net::Fd(open( incremental_filename.c_str()
				     , O_RDONLY
				     ));
	} while (!src_fd && errno == EINTR);

	if (!src_fd) {
		auto my_errno = errno;
		logger.unusual( "Archive::IncrementalAppend: "
				"Failed to open %s to append to "
				"<fd %d>: %s"
			      , incremental_filename.c_str()
			      , dst_fd.get()
			      , strerror(my_errno)
			      );
		return false;
	}

	logger.debug( "Opened file %s for reading as <fd %d>."
		    , incremental_filename.c_str()
		    , src_fd.get()
		    );

	std::uint8_t buffer[4096];
	for (;;) {
		auto size = sizeof(buffer);

		errno = 0;
		auto readres = Util::Rw::read_all( src_fd.get()
						 , buffer
						 , size
						 );
		if (!readres && errno != 0) {
			auto my_errno = errno;
			logger.unusual( "Archive::IncrementalAppend: "
					"Failed to read from <fd %d> to "
					"append to <fd %d>: %s"
				      , src_fd.get()
				      , dst_fd.get()
				      , strerror(my_errno)
				      );
			return false;
		}
		/* EOF? */
		if (size == 0)
			break;

		auto writeres = Util::Rw::write_all( dst_fd.get()
						   , buffer
						   , size
						   );
		if (!writeres) {
			auto my_errno = errno;
			logger.unusual( "Archive::IncrementalAppend: "
					"Failed to write <fd %d> after "
					"reading from <fd %d>: %s"
				      , dst_fd.get()
				      , src_fd.get()
				      , strerror(my_errno)
				      );
			return false;
		}
	}

	auto syncres = fsync(dst_fd.get());
	if (syncres < 0) {
		auto my_errno = errno;
		logger.unusual( "Archive::IncrementalAppend: "
				"Failed to sync to <fd %d>: %s"
			      , dst_fd.get()
			      , strerror(my_errno)
			      );
		return false;
	}

	src_fd.reset();
	logger.debug("Closed <fd %d>.", src_fd.get());
	dst_fd.reset();
	logger.debug("Closed <fd %d>.", dst_fd.get());

	return true;
}

}

#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/FooterJudge.hpp"
#include"Ev/ThreadPool.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include"Util/Logger.hpp"
#include"Util/Rw.hpp"

namespace Archive {

FooterJudge::Judgment
FooterJudge::judge_back( std::string filename
		       , std::uint32_t data_version
		       ) {
	auto rv = Judgment();

	do {
		rv.fd = Net::Fd(::open( filename.c_str()
				      , O_RDWR | O_APPEND
				      ));
	} while (!rv.fd && errno == EINTR);
	if (!rv.fd && errno == ENOENT) {
		/* Normal, so just log at info.  */
		logger.info( "Archive::FooterJudge: File %s does not exist, "
			     "will reupload from client."
			   , filename.c_str()
			   );
		rv.type = Reupload;
		return rv;
	}
	if (!rv.fd) {
		auto my_errno = errno;
		logger.unusual( "Archive::FooterJudge: Failed while "
				"opening %s, will reupload from client: "
				"%s"
			      , filename.c_str()
			      , strerror(my_errno)
			      );
		rv.type = Reupload;
		return rv;
	}

	auto& fd = rv.fd;
	logger.debug( "Opened file %s for read/write as <fd %d>"
		    , filename.c_str()
		    , fd.get()
		    );

	auto static constexpr footer_main_size = 8 + 4 + 2;
	auto static constexpr footer_size = footer_main_size
					  + sizeof(Sha256::Hash)
					  ;
	auto seekres = lseek(fd.get(), off_t(-footer_size), SEEK_END);
	if (seekres < off_t(0)) {
		auto my_errno = errno;
		logger.unusual( "Archive::FooterJudge: Failed while "
				"seeking <fd %d> (which will be closed), "
				"will reupload from client: "
				"%s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}

	std::uint8_t buffer[footer_size];
	auto read_size = footer_size;
	auto readres = Util::Rw::read_all(fd.get(), buffer, read_size);
	if (read_size < footer_size) {
		logger.unusual( "Archive::FooterJudge: Unexpected "
				"end-of-file while reading footer of"
				"<fd %d> (which will be closed), "
				"is somebody messing with the file?"
			      , fd.get()
			      );
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}
	if (!readres) {
		auto my_errno = errno;
		logger.unusual( "Archive::FooterJudge: Failed while "
				"reading from <fd %d> (which will be closed), "
				"will reupload from client: "
				"%s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}

	/* Check hash.  */
	auto actual_hash = Sha256::fun(buffer, footer_main_size);
	auto expected_hash = Sha256::Hash::from_buffer(buffer + footer_main_size);
	if (actual_hash != expected_hash) {
		logger.unusual( "Archive::FooterJudge: Footer hash checksum "
				"mismatch on <fd %d> (which will be closed), "
				"will reuplaod from client.");
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}

	/* Load the footer.  */
	auto prev_size = (std::uint64_t(buffer[0]) << 56)
		       + (std::uint64_t(buffer[1]) << 48)
		       + (std::uint64_t(buffer[2]) << 40)
		       + (std::uint64_t(buffer[3]) << 32)
		       + (std::uint64_t(buffer[4]) << 24)
		       + (std::uint64_t(buffer[5]) << 16)
		       + (std::uint64_t(buffer[6]) << 8)
		       + (std::uint64_t(buffer[7]) << 0)
		       ;
	auto prev_data_version = (std::uint32_t(buffer[8]) << 24)
			       + (std::uint32_t(buffer[9]) << 16)
			       + (std::uint32_t(buffer[10]) << 8)
			       + (std::uint32_t(buffer[11]) << 0)
			       ;
	auto prev_count = (std::uint16_t(buffer[12]) << 8)
			+ (std::uint16_t(buffer[13]) << 0)
			;
	if (prev_count == max_count) {
		logger.info( "Archive::FooterJudge: <fd %d> has %d "
			     "incremental updates, will close and "
			     "reupload from client."
			   , fd.get()
			   , int(prev_count) + 1
			   );
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}
	if (prev_data_version == data_version) {
		logger.info( "Archive::FooterJudge: <fd %d> has "
			     "same data_version = %ld as incoming update, "
			     "will replace most recent update."
			   , fd.get()
			   , long(data_version)
			   );
		rv.type = TruncateThenAppend;
		rv.to_remove = std::size_t(prev_size)
			     + std::size_t(footer_size)
			     ;
		return rv;
	} else if ((prev_data_version + 1) == data_version) {
		logger.info( "Archive::FooterJudge: will append update to "
			     "<fd %d>, data_version = %ld."
			   , fd.get()
			   , long(data_version)
			   );
		rv.type = Append;
		return rv;
	} else {
		logger.unusual( "Archive::FooterJudge: <fd %d> has "
				"unexpected data_version = %ld, incoming "
				"update has data_version = %ld, will close "
				"and reupload from client."
			      , fd.get()
			      , long(prev_data_version)
			      , long(data_version)
			      );
		rv.type = Reupload;
		rv.fd.reset();
		return rv;
	}
}

Ev::Io<FooterJudge::Judgment>
FooterJudge::judge(std::string filename, std::uint32_t data_version) {
	auto pstring = std::make_shared<std::string>(std::move(filename));
	return threadpool.background<FooterJudge::Judgment>([ this
							    , pstring
							    , data_version
							    ]() {
		return judge_back(std::move(*pstring), data_version);
	});
}

}

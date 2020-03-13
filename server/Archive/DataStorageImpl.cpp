#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/DataStorageImpl.hpp"
#include"Archive/FileNamer.hpp"
#include"Archive/IncrementalOnlySequence.hpp"
#include"Archive/IncrementalThenReuploadSequence.hpp"
#include"Archive/Reader.hpp"
#include"Daemon/ClientList.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Net/Fd.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

/* Truncates the given file descriptor, reducing its size by
 * the given number of bytes.
 */
bool do_truncate(Util::Logger& logger, int fd, std::size_t to_remove) {
	auto newsize = lseek(fd, -off_t(to_remove), SEEK_END);
	if (newsize < off_t(0)) {
		auto my_errno = errno;
		logger.BROKEN( "Could not seek <fd %d>; "
			       "will reupload from client: %s"
			     , fd
			     , strerror(my_errno)
			     );
		return false;
	}
	auto tres = int();
	do {
		tres = ftruncate(fd, newsize);
	} while (tres < 0 && errno == EINTR);
	if (tres < 0) {
		auto my_errno = errno;
		logger.BROKEN( "Could not truncate <fd %d>; "
			       "will reupload from client: %s"
			     , fd
			     , strerror(my_errno)
			     );
		return false;
	}
	return true;
}

}

namespace Archive {

DataStorageImpl::DataStorageImpl( Util::Logger& logger_
				, Ev::ThreadPool& threadpool_
				, Daemon::ClientList& clientlist_
				, std::uint16_t max_count
				, std::unique_ptr<Archive::FileNamer> namer_
				) : logger(logger_)
				  , threadpool(threadpool_)
				  , clientlist(clientlist_)
				  , judge(logger, threadpool, max_count)
				  , namer(std::move(namer_))
				  { }
DataStorageImpl::~DataStorageImpl() { }


Ev::Io<std::unique_ptr<Backup::IncrementalStorage>>
DataStorageImpl::request_incremental( Secp256k1::PubKey const& cid
				    , std::uint32_t data_version
				    ) {
	typedef std::unique_ptr<Backup::IncrementalStorage> RetT;

	if (!clientlist.has(cid))
		return Ev::lift_io<RetT>(nullptr);
	auto archive_filename = namer->get_archive_filename(cid);
	return judge.judge(archive_filename, data_version)
	     .then<RetT>([ this
			 , data_version
			 , archive_filename
			 , cid
			 ](Archive::FooterJudge::Judgment res) {
		auto ret = RetT();
		switch (res.type) {
		case Archive::FooterJudge::TruncateThenAppend:
			if (!do_truncate(logger, res.fd.get(), res.to_remove))
				goto reupload;
			/* Fall through.  */
		case Archive::FooterJudge::Append:
			ret = Util::make_unique<Archive::IncrementalOnlySequence>
				( logger
				, threadpool
				, data_version
				, res.count + 1
				, std::move(res.fd)
				);
			break;
		case Archive::FooterJudge::Reupload:
		reupload:
			ret = Util::make_unique<Archive::IncrementalThenReuploadSequence>
				( logger
				, threadpool
				, namer->get_temp_incremental_filename(cid)
				, namer->get_temp_reupload_filename(cid)
				, archive_filename
				, data_version
				);
			break;
		}
		return Ev::lift_io(std::move(ret));
	});
}

Ev::Io<std::unique_ptr<Backup::DataReader>>
DataStorageImpl::request_backup_data(Secp256k1::PubKey const& cid) {
	typedef std::unique_ptr<Backup::DataReader> RetT;

	/* if the CID is still connected, makes no sense for it to
	 * request backup data, and we would at least avoid issues
	 * about when there is a race between writing and reading.
	 * Test has to be done in the main thread.
	 */
	if (is_connected_cid(cid)) {
		logger.unusual( "Cannot open %s for reading, "
				"client might "
				"stil write to it."
			      , namer->get_archive_filename(cid).c_str()
			      );
		return Ev::lift_io<RetT>(nullptr);
	}
	return threadpool.background<Net::Fd>([this, cid]() {
		auto archive_filename = namer->get_archive_filename(cid);

		auto fd = Net::Fd();
		do {
			fd = Net::Fd(open( archive_filename.c_str()
					 , O_RDONLY
					 ));
		} while (!fd && errno == EINTR);
		if (!fd && errno == ENOENT) {
			logger.info( "%s not exist, cannot recover file."
				   , archive_filename.c_str()
				   );
		} else if (!fd) {
			auto my_errno = errno;
			logger.unusual( "Could not open %s for recovery: %s"
				      , archive_filename.c_str()
				      , strerror(errno)
				      );
		} else {
			logger.debug( "%s opened for recovery as <fd %d>."
				    , archive_filename.c_str()
				    , fd.get()
				    );
		}

		return fd;
	}).then<RetT>([this](Net::Fd fd) {
		if (!fd)
			return Ev::lift_io(RetT());

		return Ev::lift_io<RetT>(
			Util::make_unique<Archive::Reader>( logger
							  , threadpool
							  , std::move(fd)
							  )
		);
	});
}

}


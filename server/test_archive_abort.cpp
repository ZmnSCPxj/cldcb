#include<assert.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include"Archive/IncrementalThenReuploadSequence.hpp"
#include"Backup/ReuploadStorage.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/start.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"Util/Rw.hpp"
#include"Util/fork_test.hpp"

/* Tests that extra files created by an incomplete
 * IncrementalThenReuploadSequence will be deleted,
 * and that the original archive file is not
 * modified.
 */
int main() {
	Secp256k1::Random rand;

	/* Generate filenames.  */
	auto temp_incremental_filename = std::string("/tmp/incr.XXXXXX");
	auto temp_reupload_filename = std::string("/tmp/reup.XXXXXX");
	auto archive_filename = std::string("/tmp/arch.XXXXXX");
	temp_incremental_filename += '\0';
	temp_reupload_filename += '\0';
	archive_filename += '\0';

	/* Generate filenames.  */
	auto fd0 = mkstemp(&temp_incremental_filename[0]);
	assert(fd0 >= 0);
	auto fd1 = mkstemp(&temp_reupload_filename[0]);
	assert(fd1 >= 0);
	auto fd2 = mkstemp(&archive_filename[0]);
	assert(fd2 >= 0);
	close(fd0);
	close(fd1);

	/* Generate data for archive.  */
	std::uint8_t archive_data[12];
	for (auto i = std::size_t(0); i < sizeof(archive_data); ++i)
		archive_data[i] = rand.get();
	/* Write to archive.  */
	auto wres = Util::Rw::write_all( fd2
				       , archive_data
				       , sizeof(archive_data)
				       );
	assert(wres);
	close(fd2);

	/* Unlink temporaries.  */
	unlink(temp_incremental_filename.c_str());
	unlink(temp_reupload_filename.c_str());

	for (auto point = 0; point < 5; ++point) {
		auto res = Util::fork_test([ point
					   , temp_incremental_filename
					   , temp_reupload_filename
					   , archive_filename
					   ]() {
			Server::TermLogger logger;
			Ev::ThreadPool threadpool;
			auto seq = Archive::IncrementalThenReuploadSequence
				( logger
				, threadpool
				, temp_incremental_filename
				, temp_reupload_filename
				, archive_filename
				, 0
				);
			auto rseq = std::unique_ptr<Backup::ReuploadStorage>();
			Ev::start(Ev::lift_io(0).then<bool>([ &seq
							    , point
							    , &logger
							    ](int){
				if (point == 0)
					return Ev::lift_io(false);
				logger.info("point 1");
				auto chunk = std::vector<std::uint8_t>(16);
				memset(&chunk[0], 0, chunk.size());
				return seq.incremental_chunk(chunk)
				     .then<bool>([](bool ok) {
					assert(ok);
					return Ev::lift_io(true);
				});
			}).then<bool>([&seq, point, &logger](bool cont) {
				if (!cont || point == 1)
					return Ev::lift_io(false);
				logger.info("point 2");
				return seq.incremental_end()
				     .then<bool>([](bool ok) {
					assert(ok);
					return Ev::lift_io(true);
				});
			}).then<bool>([&seq, &rseq, point, &logger](bool cont) {
				if (!cont || point == 2)
					return Ev::lift_io(false);
				logger.info("point 3");
				return seq.get_response_storage()
				     .then<bool>([&rseq](std::unique_ptr<Backup::ReuploadStorage> res) {
					assert(res);
					rseq = std::move(res);
					return Ev::lift_io(true);
				});
			}).then<bool>([&rseq, point, &logger](bool cont) {
				if (!cont || point == 3)
					return Ev::lift_io(false);
				logger.info("point 4");
				auto chunk = std::vector<std::uint8_t>(16);
				memset(&chunk[0], 0, chunk.size());
				return rseq->reupload_chunk(chunk)
				     .then<bool>([](bool ok) {
					assert(ok);
					return Ev::lift_io(true);
				});
			}).then<int>([](bool) {
				return Ev::lift_io(0);
			}));
		});
		if (res)
			return *res;

		/* Check that temporary filenames are gone.  */
		auto tmpincr = open( temp_incremental_filename.c_str()
				   , O_RDONLY
				   );
		assert(tmpincr < 0 && errno == ENOENT);
		auto tmpreup = open( temp_reupload_filename.c_str()
				   , O_RDONLY
				   );
		assert(tmpreup < 0 && errno == ENOENT);

		/* Check that archive file is still the same.  */
		auto arch = open(archive_filename.c_str(), O_RDONLY);
		assert(arch >= 0);
		std::uint8_t curr_archive_data[sizeof(archive_data)];
		auto rsize = sizeof(archive_data);
		auto rres = Util::Rw::read_all(arch, curr_archive_data, rsize);
		close(arch);
		assert(rres);
		assert(rsize == sizeof(archive_data));
		assert(memcmp( archive_data
			     , curr_archive_data
			     , sizeof(archive_data)
			     ) == 0);
	}

	unlink(archive_filename.c_str());

	return 0;
}


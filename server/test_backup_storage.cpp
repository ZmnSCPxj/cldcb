#include<assert.h>
#include<cstdint>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<vector>
#include"Archive/FooterJudge.hpp"
#include"Archive/IncrementalOnlySequence.hpp"
#include"Archive/IncrementalThenReuploadSequence.hpp"
#include"Archive/Reader.hpp"
#include"Backup/DataReader.hpp"
#include"Backup/ReuploadStorage.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/start.hpp"
#include"Net/Fd.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"Util/fork_test.hpp"

namespace {

std::vector<std::uint8_t> generate_chunk() {
	Secp256k1::Random rand;
	auto size = std::size_t();
	do {
		size = (std::size_t(rand.get()) << 8)
		     + (std::size_t(rand.get()) << 0)
		     ;
	} while (size == 0);
	auto ret = std::vector<std::uint8_t>(size);
	for (auto& b : ret)
		b = rand.get();
	return ret;
}

}

/* Tests that archive writing is compatible with
 * reading it again.
 */
int main () {
	Secp256k1::Random rand;

	/* Generate filenames.  */
	auto temp_incremental_filename = std::string("/tmp/s.incr.XXXXXX");
	auto temp_reupload_filename = std::string("/tmp/s.reup.XXXXXX");
	auto archive_filename = std::string("/tmp/s.arch.XXXXXX");
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
	close(fd2);

	/* Generate chunks.  */
	auto reup_chunk0 = generate_chunk();
	auto reup_chunk1 = generate_chunk();
	auto incr0_chunk0 = generate_chunk();
	auto incr1_chunk0 = generate_chunk();
	auto incr1_chunk1 = generate_chunk();

	/* Initially create archive file.  */
	auto sres = Util::fork_test([ temp_incremental_filename
				    , temp_reupload_filename
				    , archive_filename
				    , reup_chunk0
				    , reup_chunk1
				    , incr0_chunk0
				    ]() {
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;
		auto seq = Archive::IncrementalThenReuploadSequence
			( logger
			, threadpool
			, temp_incremental_filename
			, temp_reupload_filename
			, archive_filename
			, 0 /* data_Version.  */
			);
		auto rseq = std::unique_ptr<Backup::ReuploadStorage>();
		Ev::start(Ev::lift_io(0).then<int>([ &seq
						   , &incr0_chunk0
						   ](int){
			return seq.incremental_chunk(incr0_chunk0)
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int){
			return seq.incremental_end()
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &rseq](int) {
			assert(seq.will_response_reupload());
			return seq.get_response_storage()
			     .then<int>([&rseq](std::unique_ptr<Backup::ReuploadStorage> res) {
				assert(res);
				rseq = std::move(res);
				return Ev::lift_io(0);
			});
		}).then<int>([&rseq, &reup_chunk0](int) {
			return rseq->reupload_chunk(reup_chunk0)
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&rseq, &reup_chunk1](int) {
			return rseq->reupload_chunk(reup_chunk1)
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&rseq](int) {
			return rseq->reupload_end()
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}));
	});
	if (sres)
		return *sres;

	/* Check incr and reup have been deleted, and that archive still
	 * exists.
	 */
	{
		auto tmpincr = open( temp_incremental_filename.c_str()
				   , O_RDONLY
				   );
		assert(tmpincr < 0 && errno == ENOENT);
		auto tmpreup = open( temp_reupload_filename.c_str()
				   , O_RDONLY
				   );
		assert(tmpreup < 0 && errno == ENOENT);
		auto tmparch = open( archive_filename.c_str()
				   , O_RDONLY
				   );
		assert(tmparch >= 0);
		close(tmparch);
	}

	/* Now emulate a further incremental update.  */
	auto ires = Util::fork_test([ archive_filename
				    , incr1_chunk0
				    , incr1_chunk1
				    ]() {
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;
		auto fd = Net::Fd(open( archive_filename.c_str()
				      , O_RDWR | O_APPEND
				      ));
		assert(fd);
		Archive::IncrementalOnlySequence seq
			( logger
			, threadpool
			, 1 /* data_version */
			, 1 /* count */
			, std::move(fd)
			);
		Ev::start(Ev::lift_io(0).then<int>([&seq, &incr1_chunk0](int) {
			return seq.incremental_chunk(incr1_chunk0)
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &incr1_chunk1](int) {
			return seq.incremental_chunk(incr1_chunk1)
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int) {
			return seq.incremental_end()
			     .then<int>([](bool ok) {
				assert(ok);
				return Ev::lift_io(0);
			});
		}));
	});
	if (ires)
		return *ires;

	/* Now finally do a read.  */
	auto rres = Util::fork_test([ archive_filename
				    , reup_chunk0
				    , reup_chunk1
				    , incr0_chunk0
				    , incr1_chunk0
				    , incr1_chunk1
				    ]() {
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;
		auto fd = Net::Fd(open( archive_filename.c_str()
				      , O_RDONLY
				      ));
		assert(fd);
		auto seq = Archive::Reader(logger, threadpool, std::move(fd));

		typedef std::unique_ptr<std::vector<std::uint8_t>> ReupRetT;
		typedef std::unique_ptr<Backup::DataReader::IncrementMsg> IncrRetT;

		Ev::start(Ev::lift_io(0).then<int>([&seq, &reup_chunk0](int) {
			return seq.backedup_reupload_chunk()
			     .then<int>([&reup_chunk0](ReupRetT res) {
				assert(res);
				assert(*res == reup_chunk0);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &reup_chunk1](int) {
			return seq.backedup_reupload_chunk()
			     .then<int>([&reup_chunk1](ReupRetT res) {
				assert(res);
				assert(*res == reup_chunk1);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int) {
			return seq.backedup_reupload_chunk()
			     .then<int>([](ReupRetT res) {
				assert(res);
				assert(res->empty());
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int) {
			return seq.backedup_incremental_get()
			     .then<int>([](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::New);
				assert(res->data_version == 0);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &incr0_chunk0](int) {
			return seq.backedup_incremental_get()
			     .then<int>([&incr0_chunk0](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::Chunk);
				assert(res->chunk == incr0_chunk0);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int) {
			return seq.backedup_incremental_get()
			     .then<int>([](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::New);
				assert(res->data_version == 1);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &incr1_chunk0](int) {
			return seq.backedup_incremental_get()
			     .then<int>([&incr1_chunk0](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::Chunk);
				assert(res->chunk == incr1_chunk0);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq, &incr1_chunk1](int) {
			return seq.backedup_incremental_get()
			     .then<int>([&incr1_chunk1](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::Chunk);
				assert(res->chunk == incr1_chunk1);
				return Ev::lift_io(0);
			});
		}).then<int>([&seq](int) {
			return seq.backedup_incremental_get()
			     .then<int>([](IncrRetT res) {
				assert(res);
				assert(res->type == Backup::DataReader::EndAll);
				return Ev::lift_io(0);
			});
		}));
	});

	unlink(archive_filename.c_str());

	return 0;
}

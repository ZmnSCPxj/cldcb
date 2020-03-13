#include<assert.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Archive/FooterJudge.hpp"
#include"Archive/IncrementalOnlySequence.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Ev/start.hpp"
#include"Net/Fd.hpp"
#include"Secp256k1/Random.hpp"
#include"Server/TermLogger.hpp"
#include"Util/fork_test.hpp"

namespace {

auto constexpr chunk_size = std::size_t(4096);

}

/* Tests that the increment writing will produce a footer
 * that will be judged correctly.
 */
int main() {
	Secp256k1::Random rand;

	/* Generate filenames.  */
	auto archive_filename = std::string("/tmp/f.arch.XXXXXX");
	archive_filename += '\0';
	auto fd = mkstemp(&archive_filename[0]);
	assert(fd >= 0);
	close(fd);

	/* Get random data_version.  */
	auto start_data_version = (std::uint32_t(rand.get()) << 24)
				+ (std::uint32_t(rand.get()) << 16)
				+ (std::uint32_t(rand.get()) << 8)
				+ (std::uint32_t(rand.get()) << 0)
				;
	/* Get random count.  */
	auto start_count = (std::uint16_t(rand.get()) << 8)
			 + (std::uint16_t(rand.get()) << 0)
			 ;
	if (start_count == 65535)
		start_count = 65534;

	/* First generate the file.  */
	auto res0 = Util::fork_test([ &archive_filename
				    , &rand
				    , start_data_version
				    , start_count
				    ]() {
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;

		auto fd = Net::Fd(open( archive_filename.c_str()
				      , O_RDWR | O_APPEND
				      ));
		assert(fd);
		Archive::IncrementalOnlySequence seq( logger
						    , threadpool
						    , start_data_version
						    , start_count
						    , std::move(fd)
						    );
		Ev::start(Ev::lift_io(0).then<bool>([&seq, &rand](int) {
			assert(!seq.will_response_reupload());
			auto chunk = std::vector<std::uint8_t>(chunk_size);
			for (auto& b : chunk)
				b = rand.get();
			return seq.incremental_chunk(std::move(chunk));
		}).then<bool>([&seq](bool res) {
			assert(res);
			return seq.incremental_end();
		}).then<int>([](bool res) {
			assert(res);
			return Ev::lift_io(0);
		}));
	});
	if (res0)
		return *res0;

	/* Now judge when data_version is equal to most recent.  */
	auto res1 = Util::fork_test([ &archive_filename
				    , start_data_version
				    , start_count
				    ]() {
		typedef Archive::FooterJudge::Judgment Judgment;
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;

		auto judge = Archive::FooterJudge(logger, threadpool, 65535);
		Ev::start(Ev::lift_io(0).then<Judgment>([ &judge
							, &archive_filename
							, start_data_version
							](int) {
			return judge.judge( archive_filename
					  , start_data_version
					  );
		}).then<int>([start_count](Judgment res) {
			assert( res.type
			     == Archive::FooterJudge::TruncateThenAppend
			      );
			assert(res.fd);
			/* 4 == data_version
			 * 2 + chunk_size == size-prefixed chunk.
			 * 2 == terminating 0-sized chunk.
			 * 8 + 4 + 2 + 32 == footer size.
			 */
			assert(res.to_remove == 4
					      + 2 + chunk_size
					      + 2
					      + 8 + 4 + 2 + 32
					      );
			assert(res.count == start_count);
			return Ev::lift_io(0);
		}));
	});
	if (res1)
		return *res1;

	/* Now judge when data_version is +1 to most recent.  */
	auto res2 = Util::fork_test([ &archive_filename
				    , start_data_version
				    , start_count
				    ]() {
		typedef Archive::FooterJudge::Judgment Judgment;
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;

		auto judge = Archive::FooterJudge(logger, threadpool, 65535);
		Ev::start(Ev::lift_io(0).then<Judgment>([ &judge
							, &archive_filename
							, start_data_version
							](int) {
			return judge.judge( archive_filename
					  , start_data_version + 1
					  );
		}).then<int>([start_count](Judgment res) {
			assert( res.type
			     == Archive::FooterJudge::Append
			      );
			assert(res.fd);
			assert(res.count == start_count);
			return Ev::lift_io(0);
		}));
	});
	if (res2)
		return *res2;

	/* Now judge when max_count is low.  */
	auto res3 = Util::fork_test([ &archive_filename
				    , start_data_version
				    ]() {
		typedef Archive::FooterJudge::Judgment Judgment;
		Server::TermLogger logger;
		Ev::ThreadPool threadpool;

		auto judge = Archive::FooterJudge( logger, threadpool
						 , 0
						 );
		Ev::start(Ev::lift_io(0).then<Judgment>([ &judge
							, &archive_filename
							, start_data_version
							](int) {
			return judge.judge( archive_filename
					  , start_data_version + 1
					  );
		}).then<int>([](Judgment res) {
			assert( res.type
			     == Archive::FooterJudge::Reupload
			      );
			return Ev::lift_io(0);
		}));
	});
	if (res3)
		return *res3;

	unlink(archive_filename.c_str());

	return 0;
}

#include<algorithm>
#include<assert.h>
#include<sstream>
#include<thread>
#include<vector>
#include"LD/Writer.hpp"
#include"Util/make_unique.hpp"


int main() {

	{
		auto constexpr number = 50;
		auto os = std::ostringstream();
		auto wr = Util::make_unique<LD::Writer>(os);

		auto threads = std::vector<std::thread>();
		for (auto i = 0; i < number; ++i) {
			threads.emplace_back([i, &wr]() {
				auto sum = 0;
				for (auto n = 0; n < i; ++n) {
					sum += n + 1;
				}
				auto my_os = std::ostringstream();
				my_os << std::dec << sum;
				wr->write(my_os.str());
			});
		}

		/* Make sure all senders complete.  */
		for (auto& t : threads)
			t.join();
		/* Now destroy the writer object, to ensure it
		 * has completed writing everything.
		 */
		wr = nullptr;

		auto s = os.str();
		/* There should be "number" lines.  */
		assert(std::count(s.begin(), s.end(), '\n') == number);
		/* All the printed lines should be present.  */
		for (auto i = 0; i < number; ++i) {
			auto sum = (i * i + i) / 2;
			auto my_os = std::ostringstream();
			my_os << std::dec << sum << std::endl;
			auto str_i = my_os.str();
			/* FIXME: Not exactly accurate, as 1\n will also
			 * match 41\n.
			 */
			assert(std::search( s.begin(), s.end()
					  , str_i.begin(), str_i.end()
					  ) != s.end());
		}
	}

	return 0;
}

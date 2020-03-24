#include<assert.h>
#include<future>
#include<queue>
#include<thread>
#include"Net/SocketFd.hpp"
#include"Net/socketpair.hpp"
#include"ServerTalker/Detail/QueueTalker.hpp"
#include"Util/Rw.hpp"
#include"Util/make_unique.hpp"

using ServerTalker::Detail::QueueTalker;

class TestItem : public QueueTalker::Item {
public:
	std::promise<char> to_load;
};

int main() {
	auto fds = Net::socketpair();
	auto& server_fd = fds.first;
	auto& client_fd = fds.second;

	QueueTalker talker(server_fd.get());

	auto server = std::thread([&talker, &server_fd]() {
		auto q = std::queue<char>();
		auto run = true;
		while (run) {
			auto res = talker.wait(QueueTalker::Read);
			switch (res.type) {
			case QueueTalker::ItemAvailable: {
				auto& item = *res.item;
				auto& test_item = static_cast<TestItem&>(item);
				if (q.empty()) {
					test_item.to_load.set_value(0);
				} else {
					test_item.to_load.set_value(q.front());
					q.pop();
				}
			} break;
			case QueueTalker::FdAvailable: {
				auto c = char();
				auto size = std::size_t(1);
				auto rres = Util::Rw::read_all
					( server_fd.get()
					, &c
					, size
					);
				assert(rres);
				q.push(c);
			} break;
			case QueueTalker::Abort:
				run = false;
			}
		}
		/* Close our end of the socket.  */
		server_fd.reset();
	});

	auto number = std::size_t(200);

	/* First loop: Send all data, then acquire them.  */
	for (auto i = 0; i < number; ++i) {
		auto c = char(i);
		auto wres = Util::Rw::write_all
			( client_fd.get()
			, &c
			, 1
			);
		assert(wres);
	}
	for (auto i = 0; i < number; ++i) {
		auto item = Util::make_unique<TestItem>();
		auto future = item->to_load.get_future();
		talker.post(std::move(item));
		assert(future.get() == char(i));
	}
	/* Second loop: send some data first, then acquire.
	 * In the below, we send two bytes before we ask for
	 * the first one.
	 */
	for (auto i = 0; i < number + 1; ++i) {
		if (i < number) {
			auto c = char(i);
			auto wres = Util::Rw::write_all
				( client_fd.get()
				, &c
				, 1
				);
			assert(wres);
		}

		if (i > 0) {
			auto j = i - 1;
			auto item = Util::make_unique<TestItem>();
			auto future = item->to_load.get_future();
			talker.post(std::move(item));
			assert(future.get() == char(j));
		}
	}

	talker.abort();
	server.join();

	return 0;
}


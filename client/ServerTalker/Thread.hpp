#ifndef CLDCB_CLIENT_SERVERTALKER_THREAD_HPP
#define CLDCB_CLIENT_SERVERTALKER_THREAD_HPP

#include<exception>
#include<functional>
#include<future>
#include<memory>

namespace ServerTalker { class MessengerIf; }
namespace Util { class Logger; }

namespace ServerTalker {

/* Represents a background thread that first
 * creates the given messenger object and
 * wraps it in a ServerTalker::Detail::DePinger,
 * then allows access to this messenger from
 * other threads.
 */
class Thread {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

	void core_submit(std::function<void(ServerTalker::MessengerIf&)>);

public:
	Thread() =delete;
	Thread(Thread&&);
	~Thread();

	explicit
	Thread( Util::Logger& logger
	      , std::function<std::unique_ptr<ServerTalker::MessengerIf>()>
			messenger_constructor
	      );

	/* Submit a task to be performed.  */
	template<typename r>
	std::future<r>
	submit(std::function<r(ServerTalker::MessengerIf&)> f) {
		auto promise = std::make_shared<std::promise<r>>();
		core_submit([promise, f](ServerTalker::MessengerIf& m) {
			try {
				promise->set_value(f(m));
			} catch (...) {
				try {
					promise->set_exception(
						std::current_exception()
					);
				} catch (...) { }
			}
		});
		return promise->get_future();
	}

};

}

#endif /* CLDCB_CLIENT_SERVERTALKER_THREAD_HPP */

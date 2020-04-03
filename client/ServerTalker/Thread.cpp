#include<condition_variable>
#include<mutex>
#include<stdexcept>
#include"ServerTalker/Detail/DePinger.hpp"
#include"ServerTalker/Detail/QueueTalker.hpp"
#include"ServerTalker/Thread.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

class ThreadItem : public ServerTalker::Detail::QueueTalker::Item {
private:
	std::function<void(ServerTalker::MessengerIf&)> function;

public:
	ThreadItem() =delete;
	explicit
	ThreadItem(std::function<void(ServerTalker::MessengerIf&)> function_
		  ) : function(std::move(function_))
		    { }

	void operator()(ServerTalker::MessengerIf& m) {
		function(m);
	}
};

}

namespace ServerTalker {

class Thread::Impl {
private:
	Util::Logger& logger;

	/* Owned completely by the background thread.  */
	std::unique_ptr<Detail::DePinger> depinger;

	/* Mutex and condvar to handle the talker pointer.  */
	std::mutex mtx;
	std::condition_variable cnd;
	std::unique_ptr<Detail::QueueTalker> talker;
	/* Protected by the above mutex as well, set if connecting failed.  */
	bool broken;

	std::thread background;

	void
	background_process( std::function<std::unique_ptr<MessengerIf>()>
					messenger_constructor
			  ) {
		logger.debug("Starting ServerTalker::Thread.  Connecting...");

		/* Connect.  */
		auto local_messenger = messenger_constructor();
		if (!local_messenger) {
			logger.BROKEN( "ServerTalker::Thread: "
				       "Failed to connect, thread is broken."
				     );
			/* Set broken flag.  */
			auto lock = std::unique_lock<std::mutex>(mtx);
			broken = true;
			cnd.notify_all();
			return;
		}
		depinger = Util::make_unique<Detail::DePinger>(
			std::move(local_messenger)
		);

		/* Construct queuetalker.  */
		auto local_talker = Util::make_unique<Detail::QueueTalker>(
			depinger->get_fd()
		);
		auto& my_talker = *local_talker;

		/* Load queuetalker into this.  */
		{
			auto lock = std::unique_lock<std::mutex>(mtx);
			talker = std::move(local_talker);
			cnd.notify_all();
		}

		logger.debug( "ServerTalker::Thread: Handshake complete, "
			      "entering main loop."
			    );
		for (;;) {
			auto res = my_talker.wait(Detail::QueueTalker::Read);
			switch (res.type) {
			case Detail::QueueTalker::ItemAvailable: {
				auto& function = static_cast<ThreadItem&>(
					*res.item
				);
				function(*depinger);
			} break;
			case Detail::QueueTalker::FdAvailable:
				depinger->deping();
				break;
			case Detail::QueueTalker::Abort:
				return;
			}
		}
	}

public:
	Impl( Util::Logger& logger_
	    , std::function<std::unique_ptr<MessengerIf>()>
			messenger_constructor
	    ) : logger(logger_)
	      , depinger()
	      , mtx()
	      , cnd()
	      , talker()
	      , broken(false)
	      , background([this, messenger_constructor]() {
			background_process(messenger_constructor);
		})
	      { }

	~Impl() {
		Detail::QueueTalker *ptalker = nullptr;
		/* Check if broken or talker is constructed.  */
		{
			auto lock = std::unique_lock<std::mutex>(mtx);
			while (!broken && !talker) {
				cnd.wait(lock);
			}
			if (!broken) {
				ptalker = &*talker;
			}
		}
		/* If talker is constructed, signal abort to the
		 * background thread.
		 */
		if (ptalker)
			ptalker->abort();
		/* Wait for background thread to complete.  */
		background.join();
	}

	void
	core_submit(std::function<void(ServerTalker::MessengerIf&)> function) {
		auto item = Util::make_unique<ThreadItem>(std::move(function));
		Detail::QueueTalker *ptalker = nullptr;
		{
			auto lock = std::unique_lock<std::mutex>(mtx);
			while (!broken && !talker) {
				cnd.wait(lock);
			}
			if (broken)
				throw std::runtime_error(
					"ServerTalker::Thread broken!"
				);
			ptalker = &*talker;
		}
		ptalker->post(std::move(item));
	}
};

Thread::Thread(Thread&& o)
	: pimpl(std::move(o.pimpl)) { }
Thread::~Thread() {}

Thread::Thread( Util::Logger& logger
	      , std::function<std::unique_ptr<ServerTalker::MessengerIf>()>
			messenger_constructor
	      ) : pimpl(Util::make_unique<Impl>(logger, std::move(messenger_constructor)))
		{ }

void Thread::core_submit(std::function<void(ServerTalker::MessengerIf&)> f) {
	if (!pimpl)
		throw std::logic_error("Use of ServerTalker::Thread after being moved from.");
	pimpl->core_submit(std::move(f));
}

}

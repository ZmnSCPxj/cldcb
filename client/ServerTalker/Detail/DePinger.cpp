#include<assert.h>
#include<queue>
#include<thread>
#include"ServerTalker/Detail/DePinger.hpp"
#include"ServerTalker/Detail/QueueTalker.hpp"
#include"ServerTalker/Messenger.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

class GenericItem : public ServerTalker::Detail::QueueTalker::Item {
public:
	/* Return non-nullptr to indicate that this is a write attempt,
	 * and the returned pointer is a pointer to the message to be
	 * sent.
	 */
	virtual Protocol::Message* write() =0;
	/* Implement if write() returns non-nullptr.
	 * This is invoked with wheether the write
	 * succeeded or not.
	 */
	virtual void write_result(bool) =0;
	/* Implement if write() returns nullptr.
	 * This is invoked with the message received.
	 */
	virtual void read_result(std::unique_ptr<Protocol::Message>) =0;
};

class SendItem : public GenericItem {
private:
	Protocol::Message message;
	std::promise<bool> result;

public:
	SendItem() =delete;
	explicit SendItem(Protocol::Message message_)
		: message(std::move(message_))
		, result()
		{ }

	std::future<bool> get_future() { return result.get_future(); }

	Protocol::Message* write() { return &message; }
	void write_result(bool res) { result.set_value(res); }
	void read_result(std::unique_ptr<Protocol::Message>) {
		assert(false);
	}
};

class ReceiveItem : public GenericItem {
private:
	std::promise<std::unique_ptr<Protocol::Message>> result;

public:
	std::future<std::unique_ptr<Protocol::Message>> get_future() {
		return result.get_future();
	}

	Protocol::Message* write() { return nullptr; }
	void write_result(bool res) { assert(false); }
	void read_result(std::unique_ptr<Protocol::Message> msg) {
		result.set_value(std::move(msg));
	}
};

}

namespace ServerTalker { namespace Detail {

class DePinger::Impl {
private:
	std::unique_ptr<ServerTalker::Messenger> messenger;
	QueueTalker queue_talker;

	std::thread background;

	void background_process() {
		auto q = std::queue<std::unique_ptr<Protocol::Message>>();
		for (;;) {
			auto result = queue_talker.wait(QueueTalker::Read);

			switch (result.type) {
			case QueueTalker::ItemAvailable: {
				auto& item = static_cast<GenericItem&>
					(*result.item);
				handle(item, q);
			} break;
			case QueueTalker::FdAvailable: {
				auto message = messenger->receive_message();
				if ( message
				  && message->id == std::uint16_t(Protocol::MID::ping)
				   )
					/* Deping!  */
					pong();
				else
					q.emplace(std::move(message));
			} break;
			case QueueTalker::Abort:
				return;
			}

		}
	}

	void handle( GenericItem& item
		   , std::queue<std::unique_ptr<Protocol::Message>>& q
		   ) {
		if (item.write()) {
			auto res = messenger->send_message(
				std::move(*item.write())
			);
			item.write_result(res);
		} else if (!q.empty()) {
			item.read_result(std::move(q.front()));
			q.pop();
		} else {
			auto message = std::unique_ptr<Protocol::Message>();
			for (;;) {
				message = messenger->receive_message();
				if ( message
				  && message->id == std::uint16_t(Protocol::MID::ping)
				   )
					pong();
				else
					break;
			}
			item.read_result(std::move(message));
		}
	}

	void pong() {
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::pong);
		/* TODO: give random TLVs for payload.  */
		messenger->send_message(std::move(msg));
	}

public:
	Impl( std::unique_ptr<ServerTalker::Messenger> messenger_
	    ) : messenger(std::move(messenger_))
	      , queue_talker(messenger->get_fd())
	      , background([this]() { background_process(); })
	      { }
	~Impl() {
		queue_talker.abort();
		background.join();
	}

	std::future<std::unique_ptr<Protocol::Message>>
	receive_message() {
		auto item = Util::make_unique<ReceiveItem>();
		auto rv = item->get_future();
		queue_talker.post(std::move(item));
		return rv;
	}
	std::future<bool>
	send_message(Protocol::Message message) {
		auto item = Util::make_unique<SendItem>(std::move(message));
		auto rv = item->get_future();
		queue_talker.post(std::move(item));
		return rv;
	}
};

DePinger::DePinger(std::unique_ptr<ServerTalker::Messenger> messenger)
	: pimpl(Util::make_unique<Impl>(std::move(messenger)))
	{ }
DePinger::~DePinger() { }
std::future<std::unique_ptr<Protocol::Message>>
DePinger::receive_message() {
	return pimpl->receive_message();
}
std::future<bool>
DePinger::send_message(Protocol::Message message) {
	return pimpl->send_message(std::move(message));
}

}}

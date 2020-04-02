#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"ServerTalker/Detail/DePinger.hpp"

namespace ServerTalker { namespace Detail {

DePinger::DePinger( std::unique_ptr<ServerTalker::MessengerIf> messenger_
		  ) : messenger(std::move(messenger_))
		    , queue()
		    { }
DePinger::~DePinger() { }

std::unique_ptr<Protocol::Message>
DePinger::receive_message() {
	while (queue.empty()) {
		deping();
	}
	auto ret = std::move(queue.front());
	queue.pop();
	return ret;
}
bool
DePinger::send_message(Protocol::Message message) {
	return messenger->send_message(std::move(message));
}
void
DePinger::deping() {
	auto msg = messenger->receive_message();
	if (msg && msg->id == std::uint16_t(Protocol::MID::ping)) {
		msg->id = Protocol::MID::pong;
		messenger->send_message(std::move(*msg));
		return;
	}
	if (msg && msg->id == std::uint16_t(Protocol::MID::pong)) {
		return;
	}
	queue.push(std::move(msg));
}

}}

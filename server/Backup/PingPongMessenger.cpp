#include"Backup/PingPongMessenger.hpp"
#include"Daemon/Messenger.hpp"
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"

namespace Backup {

Ev::Io<std::unique_ptr<Protocol::Message>>
PingPongMessenger::receive_message() {
	typedef std::unique_ptr<Protocol::Message> RetT;
	return Ev::yield().then<RetT>([this](int) {
		return messenger.receive_message(10, timedout);
	}).then<RetT>([this](RetT res) {
		if (!res)
			return Ev::lift_io(std::move(res));
		switch (Protocol::MID::Type(res->id)) {
		case Protocol::MID::ping:
		case Protocol::MID::pong:
			return receive_message();

		default:
			return Ev::lift_io(std::move(res));
		}
	});
}

Ev::Io<bool> PingPongMessenger::send_message(Protocol::Message msg) {
	return messenger.send_message(std::move(msg));
}

}

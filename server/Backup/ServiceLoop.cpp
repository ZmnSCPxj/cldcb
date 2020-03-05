#include<sstream>
#include"Backup/ServiceLoop.hpp"
#include"Backup/StorageIf.hpp"
#include"Ev/Io.hpp"
#include"Protocol/Message.hpp"
#include"Protocol/MID.hpp"
#include"Util/Logger.hpp"
#include"Util/Str.hpp"

namespace Backup {

void ServiceLoop::init() {
	storage.connect_cid(cid);
}
ServiceLoop::~ServiceLoop() {
	storage.disconnect_cid(cid);
}

Ev::Io<int>
ServiceLoop::enter_loop() {
	/* This function ensures that this object remains
	 * alive via the self-pointer.
	 */
	auto self = get_self();
	return self->loop().then<int>([self](int) {
		auto cid_string = ([](Secp256k1::PubKey const& cid) {
			auto os = std::ostringstream();
			os << cid;
			auto s = os.str();
			s[0] = 'c';
			return s;
		})(self->cid);
		self->logger.info("Disconnecting %s from <fd %d>"
				 , cid_string.c_str()
				 , self->fd_num
				 );
		return Ev::lift_io(0);
	});
}

Ev::Io<int>
ServiceLoop::loop() {
	/* Randomize timeout.  */
	return messenger.receive_message(59.1, timedout)
	     .then<int>([this](std::unique_ptr<Protocol::Message> msg) {
		if (msg)
			return dispatch_msg(std::move(*msg));
		else if (timedout)
			return ping();
		else {
			logger.debug( "Could not get message in "
				      "Backup::ServiceLoop::loop <fd %d>"
				    , fd_num
				    );
			return Ev::lift_io(0);
		}
	});
}

Ev::Io<int>
ServiceLoop::ping() {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::ping);
	/* TODO: randomize size.  */
	msg.tlvs[0] = std::vector<std::uint8_t>(42);

	return messenger.send_message(msg)
	     .then<int>([this](bool res) {
		if (!res) {
			logger.unusual( "Failed to send ping <fd %d>."
				      , fd_num
				      );
			return Ev::lift_io(0);
		}
		return loop();
	});
}

Ev::Io<int>
ServiceLoop::dispatch_msg(Protocol::Message msg) {
	switch (Protocol::MID::Type(msg.id)) {
	case Protocol::MID::pong:
		/* Ignore!  */
		return loop();
	case Protocol::MID::ping:
		return pong();

	/* FIXME: Other messages.  */

	default:
		logger.unusual( "<fd %d> got unexpected message %s"
			      , fd_num
			      , describe_msg(msg).c_str()
			      );
		return Ev::lift_io(0);
	}
}

std::string
ServiceLoop::describe_msg(Protocol::Message const& msg) {
	auto os = std::ostringstream();
	os << msg.id << " {";
	auto first = true;
	for (auto& tlv : msg.tlvs) {
		if (!first) {
			os << ", ";
		}
		first = false;

		auto t = tlv.first;
		os << t << ": ";

		auto& v = tlv.second;
		if (v.size() <= 10) {
			os << Util::Str::hexdump(&v[0], v.size());
		} else {
			os << Util::Str::hexdump(&v[0], 8)
			   << ".." << v.size()
			    ;
		}
	}
	os << " }";
	return os.str();
}

Ev::Io<int>
ServiceLoop::pong() {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::pong);
	/* TODO: randomize size.  */
	msg.tlvs[0] = std::vector<std::uint8_t>(42);

	return messenger.send_message(msg)
	     .then<int>([this](bool res) {
		if (!res) {
			logger.unusual( "Failed to send pong <fd %d>."
				      , fd_num
				      );
			return Ev::lift_io(0);
		}
		return loop();
	});
}

}

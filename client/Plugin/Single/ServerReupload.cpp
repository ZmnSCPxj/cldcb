#include"Plugin/Single/ServerReupload.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"ServerTalker/Detail/DePinger.hpp"
#include"Util/Logger.hpp"

namespace Plugin { namespace Single {

void ServerReupload::init() {
	logger.debug("Plugin::Single::ServerReupload constructed.");
}

std::future<bool>
ServerReupload::send_reupload_chunk(std::vector<std::uint8_t> ciphertext) {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::reupload_chunk);
	msg.tlvs[0] = std::move(ciphertext);
	return depinger.send_message(std::move(msg));
}
std::future<bool>
ServerReupload::reupload_completed() {
	logger.debug("Plugin::Single::ServerReupload completing.");
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::reupload_end);
	auto ret1 = std::make_shared<std::future<bool>>(
		 depinger.send_message(std::move(msg))
	);

	return std::async([ret1, this]() {
		auto send_res = ret1.get();
		if (!send_res)
			return false;

		auto msg = depinger.receive_message().get();
		if (!msg) {
			logger.BROKEN( "Server disconnected at "
				       "end of reupload."
				     );
			return false;
		}
		if (msg->id == std::uint16_t(Protocol::MID::acknowledge_upload))
			return true;

		logger.BROKEN( "Server sent unexpected message id %d."
			     , (int) msg->id
			     );

		return false;
	});
}

}}

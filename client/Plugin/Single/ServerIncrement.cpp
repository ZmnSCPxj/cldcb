#include"Plugin/ServerResult.hpp"
#include"Plugin/Single/ServerIncrement.hpp"
#include"Plugin/Single/ServerReupload.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"ServerTalker/Detail/DePinger.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Plugin { namespace Single {

void ServerIncrement::init() {
	logger.debug("Plugin::Single::ServerIncrement constructed.");
}

std::future<bool>
ServerIncrement::send_increment_chunk(std::vector<std::uint8_t> ciphertext) {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::incremental_chunk);
	msg.tlvs[0] = std::move(ciphertext);
	return depinger.send_message(std::move(msg));
}

std::future<Plugin::ServerResult>
ServerIncrement::increment_completed() {
	logger.debug("Plugin::Single::ServerIncrement completing.");
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::incremental_end);
	auto ret1 = std::make_shared<std::future<bool>>(
		depinger.send_message(std::move(msg))
	);
	
	return std::async([this, ret1]() {
		auto ret = Plugin::ServerResult::failure();
		auto send_res = ret1->get();
		if (!send_res)
			return ret;

		auto msg = depinger.receive_message().get();
		if (!msg) {
			logger.BROKEN( "Server disconnected at "
				       "end of incremental update."
				     );
			return ret;
		}
		if (msg->id == std::uint16_t(Protocol::MID::acknowledge_upload)) {
			ret = Plugin::ServerResult::success();
			return ret;
		}
		if (msg->id == std::uint16_t(Protocol::MID::response_reupload)) {
			ret = Plugin::ServerResult::reupload(
				Util::make_unique<ServerReupload>( logger
								 , depinger
								 )
			);
			return ret;
		}

		logger.BROKEN( "Server sent unexpected message id %d."
			     , (int) msg->id
			     );

		return ret;
	});
}

}}

#include"Plugin/Single/ServerReupload.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"ServerTalker/MessengerIf.hpp"
#include"ServerTalker/Thread.hpp"
#include"Util/Logger.hpp"

namespace Plugin { namespace Single {

void ServerReupload::init() {
	logger.debug("Plugin::Single::ServerReupload constructed.");
}

std::future<bool>
ServerReupload::send_reupload_chunk(std::vector<std::uint8_t> ciphertext) {
	return server_thread
	     .submit<bool>([ciphertext](ServerTalker::MessengerIf& messenger) {
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::reupload_chunk);
		msg.tlvs[0] = std::move(ciphertext);

		return messenger.send_message(msg);
	});
}
std::future<bool>
ServerReupload::reupload_completed() {
	logger.debug("Plugin::Single::ServerReupload completing.");

	return server_thread
	     .submit<bool>([this](ServerTalker::MessengerIf& messenger) {
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::reupload_end);

		if (!messenger.send_message(msg))
			return false;

		auto response = messenger.receive_message();
		if (!response) {
			logger.BROKEN( "Server disconnected at "
				       "end of reupload."
				     );
			return false;
		}
		if ( response->id
		  == std::uint16_t(Protocol::MID::acknowledge_upload)
		   )
			return true;

		logger.BROKEN( "Server sent unexpected message id %d."
			     , (int) response->id
			     );

		return false;
	});
}

}}

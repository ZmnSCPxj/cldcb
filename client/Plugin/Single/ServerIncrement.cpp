#include"Plugin/ServerResult.hpp"
#include"Plugin/Single/ServerIncrement.hpp"
#include"Plugin/Single/ServerReupload.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"ServerTalker/MessengerIf.hpp"
#include"ServerTalker/Thread.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Plugin { namespace Single {

void ServerIncrement::init() {
	logger.debug("Plugin::Single::ServerIncrement constructed.");
}

std::future<bool>
ServerIncrement::send_increment_chunk(std::vector<std::uint8_t> ciphertext) {
	return server_thread
	     .submit<bool>([ciphertext](ServerTalker::MessengerIf& messenger) {
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::incremental_chunk);
		msg.tlvs[0] = std::move(ciphertext);
		return messenger.send_message(std::move(msg));
	});
}

std::future<Plugin::ServerResult>
ServerIncrement::increment_completed() {
	logger.debug("Plugin::Single::ServerIncrement completing.");

	return server_thread
	     .submit<Plugin::ServerResult>
			([this](ServerTalker::MessengerIf& messenger) {
		auto ret = Plugin::ServerResult::failure();

		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::incremental_end);
		if (!messenger.send_message(std::move(msg)))
			return ret;
	
		auto response = messenger.receive_message();
		if (!response) {
			logger.BROKEN( "Server disconnected at "
				       "end of incremental update."
				     );
			return ret;
		}
		if ( response->id
		  == std::uint16_t(Protocol::MID::acknowledge_upload)
		   ) {
			ret = Plugin::ServerResult::success();
			return ret;
		}
		if ( response->id
		  == std::uint16_t(Protocol::MID::response_reupload)
		   ) {
			ret = Plugin::ServerResult::reupload(
				Util::make_unique<ServerReupload>( logger
								 , server_thread
								 )
			);
			return ret;
		}

		logger.BROKEN( "Server sent unexpected message id %d."
			     , (int) response->id
			     );

		return ret;
	});
}

}}

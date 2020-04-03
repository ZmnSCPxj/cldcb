#include"Plugin/ServerIncrementIf.hpp"
#include"Plugin/Single/Server.hpp"
#include"Plugin/Single/ServerIncrement.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"S.hpp"
#include"ServerTalker/Messenger.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Plugin { namespace Single {

std::future<std::unique_ptr<Plugin::ServerIncrementIf>>
Server::new_update( std::uint32_t data_version
		  ) {
	return server_thread
	     .submit<std::unique_ptr<Plugin::ServerIncrementIf>>
			([ this
			 , data_version
			 ](ServerTalker::MessengerIf& messenger) {
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(Protocol::MID::request_incremental);
		msg.tlvs[0] = std::vector<uint8_t>();
		S::serialize(msg.tlvs[0], data_version);

		if (!messenger.send_message(std::move(msg))) {
			return std::unique_ptr<Plugin::ServerIncrementIf>(
				nullptr
			);
		}

		return std::unique_ptr<Plugin::ServerIncrementIf>(
			Util::make_unique<ServerIncrement>( logger
							  , server_thread
							  )
		);
	});
}

}}

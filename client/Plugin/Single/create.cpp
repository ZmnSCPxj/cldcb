#include"Plugin/ServerSpec.hpp"
#include"Plugin/Setup.hpp"
#include"Plugin/Single/Server.hpp"
#include"Plugin/Single/create.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"ServerTalker/MessengerIf.hpp"
#include"ServerTalker/connect_then_handshake.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Plugin { namespace Single {

std::unique_ptr<Plugin::ServerIf>
create( Util::Logger& logger
      , Net::Connector& connector
      , Plugin::Setup const& setup
      , Plugin::ServerSpec const& server_spec
      ) {
	auto rv = Util::make_unique<Plugin::Single::Server>(logger, [&]() {
		// TODO: pass some kind of ref to keypair.
		auto our_keypair = Secp256k1::KeyPair(setup.our_priv_key);

		auto messenger = ServerTalker::connect_then_handshake
				( logger
				, connector
				, our_keypair
				, server_spec.id
				, server_spec.host
				, server_spec.port
				);

		/* Send our nsig.  */
		auto nsig_msg = Protocol::Message();
		nsig_msg.id = std::uint16_t(Protocol::MID::give_recognition_code);
		nsig_msg.tlvs[0] = std::vector<std::uint8_t>(64);
		setup.node_sig.to_buffer(&nsig_msg.tlvs[0][0]);

		auto send_res = messenger->send_message(std::move(nsig_msg));
		if (!send_res) {
			logger.BROKEN( "Plugin::Single::create: Could "
				       "not send node signature.");
			messenger = nullptr;
		}

		return messenger;
	});

	return rv;
}

}}

#include<algorithm>
#include<iterator>
#include"Backup/RequestRecognitionCodes.hpp"
#include"Backup/StorageIf.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"Util/Logger.hpp"

namespace Backup {

Ev::Io<int>
RequestRecognitionCodes::run() {
	start_time = Ev::now();
	logger.debug( "Handling `request_recognition_codes` on <fd %d>."
		    , messenger.get_fd()
		    );

	/* Keeps self alive via self-pointer.  */
	auto self = shared_from_this();
	return core_run().then<int>([self](bool ok) {
		if (ok) {
			self->logger.debug( "Handled "
					    "`request_recognition_codes` "
					    "on <fd %d> in %f seconds."
					  , self->messenger.get_fd()
					  , Ev::now() - self->start_time
					  );
			return self->enter_loop();
		} else
			return Ev::lift_io(0);
	});
}

Ev::Io<bool>
RequestRecognitionCodes::core_run() {
	return storage.request_recognition_codes()
	     .then<bool>([this](std::vector<std::vector<std::uint8_t>> res) {
		codes = res;
		return send_loop();
	});
}
Ev::Io<bool>
RequestRecognitionCodes::send_loop() {
	return Ev::yield().then<bool>([this](int) {
		if (codes.empty()) {
			auto msg = Protocol::Message();
			msg.id = std::uint16_t(
				Protocol::MID::response_recognition_codes_end
			);
			return messenger.send_message(std::move(msg));
		}
		/* Split into batches of 675 codes.
		 * Why 675?
		 * Each code is 97 bytes, 97 * 675 = 65475, adding one more
		 * will result in 65572 which exceeds our message size.
		 */
		auto to_send = std::vector<std::vector<std::uint8_t>>();
		if (codes.size() > 675) {
			to_send.resize(675);
			std::move( codes.begin(), codes.begin() + 675
				 , to_send.begin()
				 );
			codes.erase(codes.begin(), codes.begin() + 675);
		} else
			/* Get them all.  */
			to_send = std::move(codes);

		auto to_send_size = to_send.size();

		/* Build the message.  */
		auto msg = Protocol::Message();
		msg.id = std::uint16_t(
			Protocol::MID::response_recognition_codes
		);
		msg.tlvs[0] = std::vector<std::uint8_t>();
		for (auto& code : to_send)
			/* Append.  */
			std::copy( code.begin(), code.end()
				 , std::back_inserter(msg.tlvs[0])
				 );
		return messenger.send_message(std::move(msg))
		     .then<bool>([this, to_send_size](bool ok) {
			if (!ok)
				return Ev::lift_io(false);
			logger.debug( "Sent %d recognition codes to <fd %d>."
				    , int(to_send_size)
				    , messenger.get_fd()
				    );
			return send_loop();
		});
	});
}

}


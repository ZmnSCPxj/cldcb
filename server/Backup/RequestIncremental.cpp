#include<sstream>
#include"Backup/IncrementalStorage.hpp"
#include"Backup/RequestIncremental.hpp"
#include"Backup/ReuploadStorage.hpp"
#include"Backup/StorageIf.hpp"
#include"Daemon/Messenger.hpp"
#include"Ev/Io.hpp"
#include"Ev/Join.hpp"
#include"Ev/concurrent.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Backup {

RequestIncremental::RequestIncremental( Util::Logger& logger_
				      , Daemon::Messenger& messenger_
				      , Backup::StorageIf& storage_
				      , std::function<Ev::Io<int>()> enter_loop_
				      ) : logger(logger_)
					, messenger(messenger_)
					, storage(storage_)
					, enter_loop(std::move(enter_loop_))
					, incremental_storage()
					, reupload_storage()
					, joiner()
					, start_time(0.0)
					, timedout(false)
					{ }
RequestIncremental::~RequestIncremental() { }

Ev::Io<int> RequestIncremental::run( Secp256k1::PubKey const& cid
				   , std::uint32_t data_version
				   ) {
	start_time = Ev::now();
	logger.debug( "Handling request_incremental on <fd %d> "
		      "with data_version = %ld"
		    , messenger.get_fd()
		    , long(data_version)
		    );

	auto self = shared_from_this();
	return storage.request_incremental(cid, data_version)
	     .then<int>([ self
			](std::unique_ptr<Backup::IncrementalStorage> res) {
		if (!res) {
			self->logger.BROKEN( "Client on <fd %d> is not "
					     "allowed to upload "
					     "incremental data."
					   , self->messenger.get_fd()
					   );
			return Ev::lift_io(0);
		}
		self->incremental_storage = std::move(res);
		return self->check_need_reupload();
	});
}

Ev::Io<int> RequestIncremental::check_need_reupload() {
	auto self = shared_from_this();
	if (incremental_storage->will_response_reupload()) {
		/* Need to set up a Join.  */
		joiner = Util::make_unique<Ev::Join>(2);

		/* Send the respons_reupload in the background.  */
		return Ev::concurrent(send_response_reupload())
		     .then<bool>([self](int) {
			return self->read_incremental();
		}).then<bool>([self](bool ok) {
			/* Need to join always,
			 * after send_response_reupload.
			 */
			return self->joiner->join()
			     .then<bool>([self, ok](int) {
				return Ev::lift_io(ok);
			});
		}).then<bool>([self](bool ok) {
			if (!ok)
				return Ev::lift_io(false);

			self->joiner = nullptr;

			/* Get reupload storage.  */
			return self->incremental_storage->get_response_storage()
			     .then<bool>([ self
					 ](std::unique_ptr<Backup::ReuploadStorage> res) {
				if (!res)
					return Ev::lift_io(false);
				self->incremental_storage = nullptr;
				self->reupload_storage = std::move(res);
				return Ev::lift_io(true);
			});
		}).then<bool>([self](bool ok) {
			if (!ok)
				return Ev::lift_io(false);
			/* Now reupload.  */
			return self->read_reupload();
		}).then<bool>([self](bool ok) {
			if (!ok)
				return Ev::lift_io(false);
			self->reupload_storage = nullptr;
			return self->send_acknowledge_upload();
		}).then<int>([self](bool ok) {
			return self->end(ok);
		});
	} else {
		return read_incremental().then<bool>([self](bool ok) {
			if (!ok)
				return Ev::lift_io(false);
			self->incremental_storage = nullptr;
			return self->send_acknowledge_upload();
		}).then<int>([self](bool ok) {
			return self->end(ok);
		});
	}
}

Ev::Io<int> RequestIncremental::send_response_reupload() {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::response_reupload);
	return messenger.send_message(std::move(msg))
	     .then<int>([this](bool ok) {
		if (!ok) {
			logger.unusual( "Backup::RequestIncremental: "
					"Failed to send response_reupload "
					"on <fd %d>"
				      , messenger.get_fd()
				      );
		}
		return joiner->join();
	});
}

Ev::Io<bool> RequestIncremental::read_incremental() {
	assert(incremental_storage);
	return Ev::yield()
	     .then<std::unique_ptr<Protocol::Message>>([this](int) {
		return messenger.receive_message(5.0, timedout);
	}).then<bool>([this](std::unique_ptr<Protocol::Message> msg) {
		if (!msg)
			return Ev::lift_io(false);
		switch (Protocol::MID::Type(msg->id)) {
		case Protocol::MID::incremental_chunk: {
			auto it = msg->tlvs.find(std::uint8_t(0));
			if (it == msg->tlvs.end()) {
				logger.BROKEN( "Backup::RequestIncremental: "
					       "incremental_chunk from "
					       "<fd %d> has no "
					       "chunk tlv 0"
					     , messenger.get_fd()
					     );
				return Ev::lift_io(false);
			}
			return incremental_storage->incremental_chunk(
				std::move(msg->tlvs[0])
			).then<bool>([this](bool ok) {
				if (!ok)
					return Ev::lift_io(false);
				return read_incremental();
			});
		}

		case Protocol::MID::incremental_end:
			return incremental_storage->incremental_end();

		default:
			logger.BROKEN( "Backup::RequestIncremental: "
				       "Unexpected message %d from <fd %d> "
				       "while getting incremental update."
				     , int(msg->id)
				     , messenger.get_fd()
				     );
			return Ev::lift_io(false);
		}
	});
}

Ev::Io<bool> RequestIncremental::read_reupload() {
	assert(reupload_storage);
	return Ev::yield()
	     .then<std::unique_ptr<Protocol::Message>>([this](int) {
		return messenger.receive_message(5.0, timedout);
	}).then<bool>([this](std::unique_ptr<Protocol::Message> msg) {
		if (!msg)
			return Ev::lift_io(false);
		switch (Protocol::MID::Type(msg->id)) {
		case Protocol::MID::reupload_chunk: {
			auto it = msg->tlvs.find(std::uint8_t(0));
			if (it == msg->tlvs.end()) {
				logger.BROKEN( "Backup::RequestIncremental: "
					       "reupload_chunk from "
					       "<fd %d> has no "
					       "chunk tlv 0"
					     , messenger.get_fd()
					     );
				return Ev::lift_io(false);
			}
			return reupload_storage->reupload_chunk(
				std::move(msg->tlvs[0])
			).then<bool>([this](bool ok) {
				if (!ok)
					return Ev::lift_io(false);
				return read_reupload();
			});
		}

		case Protocol::MID::reupload_end:
			return reupload_storage->reupload_end();

		default:
			logger.BROKEN( "Backup::RequestIncremental: "
				       "Unexpected message %d from <fd %d> "
				       "while getting reupload."
				     , int(msg->id)
				     , messenger.get_fd()
				     );
			return Ev::lift_io(false);
		}
	});
}

Ev::Io<bool> RequestIncremental::send_acknowledge_upload() {
	auto msg = Protocol::Message();
	msg.id = std::uint16_t(Protocol::MID::acknowledge_upload);
	return messenger.send_message(std::move(msg))
	     .then<bool>([this](bool ok) {
		if (!ok) {
			logger.unusual( "Backup::RequestIncremental: "
					"Failed to send acknowledge_upload "
					"on <fd %d>"
				      , messenger.get_fd()
				      );
		}
		return Ev::lift_io(ok);
	});
}

Ev::Io<int> RequestIncremental::end(bool ok) {
	if (!ok) {
		logger.BROKEN( "Disconnecting <fd %d>."
			     , messenger.get_fd()
			     );
		return Ev::lift_io(0);
	}
	logger.debug( "Handled `request_incremental` in <fd %d> "
		      "in %f seconds."
		    , messenger.get_fd()
		    , Ev::now() - start_time
		    );
	auto my_enter_loop = std::move(enter_loop);
	return my_enter_loop();
}

}


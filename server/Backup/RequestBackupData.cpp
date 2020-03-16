#include<assert.h>
#include<sstream>
#include"Backup/DataStorage.hpp"
#include"Backup/RequestBackupData.hpp"
#include"Ev/Io.hpp"
#include"Ev/now.hpp"
#include"Ev/yield.hpp"
#include"Protocol/MID.hpp"
#include"Protocol/Message.hpp"
#include"S.hpp"
#include"Util/Logger.hpp"

namespace Backup {

Ev::Io<int> RequestBackupData::run(Secp256k1::PubKey const& cid) {
	start_time = Ev::now();

	auto cid_string = ([cid]() {
		auto os = std::ostringstream();
		os << cid;
		auto str = os.str();
		str[0] = 'c';
		return str;
	})();
	logger.debug( "Handling `request_backup_data` on <fd %d> with "
		      "cid = %s."
		    , messenger.get_fd()
		    , cid_string.c_str()
		    );

	auto self = shared_from_this();
	return storage.request_backup_data(cid)
	     .then<int>([ self
			, cid_string
			](std::unique_ptr<Backup::DataReader> res) {
		if (!res) {
			self->logger.unusual( "No data available for cid %s"
					      "on <fd %d>, will disconnect."
					    , cid_string.c_str()
					    , self->messenger.get_fd()
					    );
			return Ev::lift_io(0);
		}
		self->reader = std::move(res);
		return self->reup_loop().then<int>([self](bool ok) {
			if (!ok)
				return Ev::lift_io(0);
			self->logger.debug( "Handled `request_backup_data` "
					    "on <fd %d> in %f seconds"
					  , self->messenger.get_fd()
					  , Ev::now() - self->start_time
					  );
			return self->enter_loop();
		});
	});
}

Ev::Io<bool>
RequestBackupData::reup_loop() {
	return Ev::yield()
	     .then<std::unique_ptr<std::vector<std::uint8_t>>>([this](int) {
		return reader->backedup_reupload_chunk();
	}).then<bool>([this](std::unique_ptr<std::vector<std::uint8_t>> res) {
		if (!res) {
			logger.unusual( "Failed while recovering "
					"reupload data for <fd %d>."
				      , messenger.get_fd()
				      );
			return Ev::lift_io(false);
		}
		assert(res->size() < (65535 - 2 - 3 - 3));

		auto msg = Protocol::Message();

		if (res->size() == 0) {
			/* End of reupload data.  */
			msg.id = std::uint16_t(
				Protocol::MID::response_backedup_reupload_end
			);
			return messenger.send_message(std::move(msg))
			     .then<bool>([this](bool ok) {
				if (!ok) {
					logger.unusual( "Failed while "
							"sending "
							"reupload end "
							"on <fd %d>."
						      , messenger.get_fd()
						      );
					return Ev::lift_io(false);
				}
				return incr_loop();
			});
		}

		msg.id = std::uint16_t(
			Protocol::MID::response_backedup_reupload_chunk
		);
		msg.tlvs[0] = std::move(*res);
		return messenger.send_message(std::move(msg))
		     .then<bool>([this](bool ok) {
			if (!ok) {
				logger.unusual( "Failed while "
						"sending "
						"reupload chunk "
						"on <fd %d>."
					      , messenger.get_fd()
					      );
				return Ev::lift_io(false);
			}
			return reup_loop();
		});
	}); 
}

Ev::Io<bool>
RequestBackupData::incr_loop() {
	typedef Backup::DataReader::IncrementMsg IncrementMsg;
	return Ev::yield()
	     .then<std::unique_ptr<IncrementMsg>>([this](int) {
		return reader->backedup_incremental_get();
	}).then<bool>([this](std::unique_ptr<IncrementMsg> res) {
		if (!res) {
			logger.unusual( "Failed while recovering "
					"incremental data from "
					"<fd %d>."
				      , messenger.get_fd()
				      );
			return Ev::lift_io(false);
		}

		auto msg = Protocol::Message();
		auto cont_incr = true;

		switch (res->type) {
		case Backup::DataReader::New:
			msg.id = std::uint16_t(
				Protocol::MID::response_backedup_incremental_new
			);
			msg.tlvs[0] = std::vector<std::uint8_t>();
			S::serialize(msg.tlvs[0], res->data_version);
			break;

		case Backup::DataReader::Chunk:
			msg.id = std::uint16_t(
				Protocol::MID::response_backedup_incremental_chunk
			);
			msg.tlvs[0] = std::move(res->chunk);
			break;

		case Backup::DataReader::EndAll:
			msg.id = std::uint16_t(
				Protocol::MID::response_backedup_incremental_endall
			);
			cont_incr = false;
			break;
		}

		return messenger.send_message(std::move(msg))
		     .then<bool>([this, cont_incr](bool ok) {
			if (!ok) {
				logger.unusual( "Failed to send incremental "
						"data on <fd %d>."
					      , messenger.get_fd()
					      );
				return Ev::lift_io(false);
			}
			if (cont_incr)
				return incr_loop();
			else
				return Ev::lift_io(true);
		});
	});
}

}

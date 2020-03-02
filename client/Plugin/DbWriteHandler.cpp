#include<assert.h>
#include<stdexcept>
#include"Crypto/Box/Sealer.hpp"
#include"LD/DbWrite.hpp"
#include"Plugin/DbFileReader.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/ServerIf.hpp"
#include"Plugin/ServerIncrementIf.hpp"
#include"Plugin/ServerResult.hpp"
#include"Plugin/Setup.hpp"
#include"Util/make_unique.hpp"

namespace Plugin {

class DbWriteHandler::Impl {
private:
	Setup& setup;
	ServerIf& server;
	DbFileReader& db;
public:
	Impl() =delete;
	Impl(Setup& setup_, ServerIf& server_, DbFileReader& db_)
		: setup(setup_), server(server_), db(db_) { }

	bool handle(LD::DbWrite const& params) {
		auto res = server.new_update(params.data_version).get();
		if (res.is_failure()) {
			return false;
		}

		/* Will be filled up later depending on whether
		 * this is an incrrement result or a reupload result.
		 */
		auto serv_incr_uptr = std::unique_ptr<ServerIncrementIf>();
		auto serv_incr_ptr = (ServerIncrementIf*)nullptr;

		if (res.is_reupload()) {
			auto sealer = Crypto::Box::Sealer(setup.node_id);
			auto& serv_reup = *res.is_reupload();

			/* Save client privkey and current data_version.  */
			{
				/* FIXME: Proper datatype and serialization.  */
				auto plaintext = std::vector<std::uint8_t>(32);
				setup.our_priv_key.to_buffer(&plaintext[0]);

				S::serialize(plaintext, params.data_version);

				/* Encrypt and send to server.  */
				auto ciphertext = sealer.seal(plaintext);
				if (!serv_reup.send_reupload_chunk(ciphertext).get())
					return false;
			}

			/* Send database.  */
			auto db_read = db.start();
			for (;;) {
				/* FIXME: Add proper constants for 16 (size
				 * of MAC).
				 */
				auto plaintext = db_read->read(65000 - 16);
				if (plaintext.size() == 0)
					break;
				auto ciphertext = sealer.seal(std::move(plaintext));
				if (!serv_reup.send_reupload_chunk(std::move(ciphertext)).get())
					return false;
			}
			db_read = nullptr;

			/* Signal completion of reupload.
			 * Now check the increment interface.
			 */
			serv_incr_uptr = serv_reup.reupload_completed().get();
			if (!serv_incr_ptr)
				return false;
			serv_incr_ptr = serv_incr_uptr.get();
		} else {
			assert(res.is_increment());
			serv_incr_ptr = res.is_increment();
		}

		auto& serv_incr = *serv_incr_ptr;

		auto plaintext = std::vector<std::uint8_t>();
		S::serialize(plaintext, params);

		/* Encrypt to the node ID.  */
		auto sealer = Crypto::Box::Sealer(setup.node_id);

		/* Send to the server.  */
		auto ptr = plaintext.begin();
		auto next = plaintext.begin();
		while (ptr < plaintext.end()) {
			auto static constexpr maxsize = (65000 - 33 - 16);
			/* Determine how much to send.  */
			if ((plaintext.end() - ptr) < maxsize) {
				next = plaintext.end();
			} else {
				next = ptr + maxsize;
			}
			/* Cut a copy of the portion to encrypt.  */
			auto to_encrypt = std::vector<std::uint8_t>(ptr, next);
			/* Encrypt.  */
			auto ciphertext = sealer.seal(std::move(to_encrypt));
			assert(ciphertext.size() < 65000);

			/* Send data.  */
			auto res = serv_incr.send_increment_chunk(std::move(ciphertext));
			if (!res.get())
				return false;

			/* Advance.  */
			ptr = next;
		}
		/* Signal completion of increment.  */
		return serv_incr.increment_completed().get();
	}
};

DbWriteHandler::DbWriteHandler(DbWriteHandler&& o) {
	o.pimpl.swap(pimpl);
}
DbWriteHandler::~DbWriteHandler() { }
DbWriteHandler::DbWriteHandler( Setup& setup
			      , ServerIf& server
			      , DbFileReader& db
			      )
	: pimpl(Util::make_unique<Impl>(setup, server, db)) { }

bool DbWriteHandler::handle(LD::DbWrite const& params) {
	if (!pimpl)
		throw std::logic_error("Use of Plugin::DbWriteHandler after being moved from.");
	return pimpl->handle(params);
}

}

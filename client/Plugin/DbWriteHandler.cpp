#include<stdexcept>
#include"Crypto/Box/Sealer.hpp"
#include"LD/DbWrite.hpp"
#include"Plugin/DbFileReader.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/ServerIf.hpp"
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
		auto plaintext = std::vector<std::uint8_t>();
		S::serialize(plaintext, params);

		/* Encrypt to the node ID.  */
		auto sealer = Crypto::Box::Sealer(setup.node_id);
		auto ciphertext = sealer.seal(plaintext);

		/* Send to the server.  */
		auto res = server.send(params.data_version, ciphertext).get();

		/* Check result.  */
		if (res.is_success())
			return true;
		auto serv_reup_ptr = res.is_reupload();
		if (!serv_reup_ptr)
			return false;

		/* Send database.  */
		{
			/* Construct a new sealer.  */
			auto sealer = Crypto::Box::Sealer(setup.node_id);
			auto& serv_reup = *serv_reup_ptr;

			/* Save client privkey and current data_version.  */
			{
				/* FIXME: Proper datatype and serialization.  */
				auto plaintext = std::vector<std::uint8_t>(36);
				setup.our_priv_key.to_buffer(&plaintext[0]);
				S::serialize(plaintext, params.data_version);
				/* Encrypt and send to server.  */
				auto ciphertext = sealer.seal(plaintext);
				if (!serv_reup.send_chunk(ciphertext).get())
					return false;
			}

			/* Send database.  */
			auto db_read = db.start();
			for (;;) {
				/* FIXME: Add proper constants for 16 (size
				 * of MAC).
				 */
				auto plaintext = db_read->read(65535 - 16);
				if (plaintext.size() == 0)
					break;
				auto ciphertext = sealer.seal(plaintext);
				if (!serv_reup.send_chunk(ciphertext).get())
					return false;
			}
			db_read = nullptr;

			serv_reup.completed().get();
		}

		return true;
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

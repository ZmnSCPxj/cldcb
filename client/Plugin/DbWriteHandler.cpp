#include<stdexcept>
#include"Crypto/Box/Sealer.hpp"
#include"LD/DbWrite.hpp"
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
public:
	Impl() =delete;
	Impl(Setup& setup_, ServerIf& server_)
		: setup(setup_), server(server_) { }

	bool handle(LD::DbWrite const& params) {
		auto plaintext = std::vector<std::uint8_t>();
		S::serialize(plaintext, params);

		/* Encrypt to the node ID.  */
		auto sealer = Crypto::Box::Sealer(setup.node_id);
		auto ciphertext = sealer.seal(plaintext);

		/* Send to the server.  */
		auto fres = server.send(params.data_version, ciphertext);
		auto res = fres.get();
		if (res.is_success())
			return true;
		auto serv_reup_ptr = res.is_reupload();
		if (!serv_reup_ptr)
			return false;
		auto& serv_reup = *serv_reup_ptr;

		/* TODO: upload the database.  */
		serv_reup.completed().get();

		return true;
	}
};

DbWriteHandler::DbWriteHandler(DbWriteHandler&& o) {
	o.pimpl.swap(pimpl);
}
DbWriteHandler::~DbWriteHandler() { }
DbWriteHandler::DbWriteHandler(Setup& setup, ServerIf& server)
	: pimpl(Util::make_unique<Impl>(setup, server)) { }

bool DbWriteHandler::handle(LD::DbWrite const& params) {
	if (!pimpl)
		throw std::logic_error("Use of Plugin::DbWriteHandler after being moved from.");
	return pimpl->handle(params);
}

}

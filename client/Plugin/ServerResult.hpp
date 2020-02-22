#ifndef CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP

#include<memory>
#include"Plugin/ServerReuploadIf.hpp"

namespace Plugin {

/* The result of sending an incremental change to the
 * server.
 * This can be either:
 * * A successful result, the server has backed up the
 *   incremental change.
 * * A reupload result, the server wants the plugin to
 *   upload the complete file prior to the most recent
 *   sent change.
 * * A failure result, the server is unable to back up.
 */
class ServerResult {
private:
	std::shared_ptr<Plugin::ServerReuploadIf> reupload_ptr;
	bool succeeded;
public:
	ServerResult() : reupload_ptr(nullptr), succeeded(false) { }
	ServerResult(ServerResult&&) =default;
	ServerResult& operator=(ServerResult&&) =default;

	static
	ServerResult success() {
		auto ret = ServerResult();
		ret.succeeded = true;
		return ret;
	}
	static
	ServerResult failure() {
		return ServerResult();
	}
	static
	ServerResult reupload(std::unique_ptr<Plugin::ServerReuploadIf> reupload) {
		auto ret = ServerResult();
		ret.reupload_ptr = std::move(reupload);
		return ret;
	}

	bool is_success() const {
		return !reupload_ptr && succeeded;
	}
	ServerReuploadIf* is_reupload() const {
		return reupload_ptr.get();
	}
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP */

#ifndef CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP

#include<memory>
#include"Plugin/ServerReuploadIf.hpp"

namespace Plugin {

/* The result of sending an incremental change to the
 * server.
 * This can be either:
 * * A success result, the plugin can now return to the
 *   lightningd.
 * * A failure result, the server is unable to back up.
 * * A reupload result, the server wants the plugin to
 *   upload the complete file prior to the most recent
 *   change, and *then* the current incremental change.
 */
class ServerResult {
private:
	std::shared_ptr<Plugin::ServerReuploadIf> reupload_ptr;
	bool succeeded;
public:
	ServerResult() : reupload_ptr(nullptr), succeeded(false) { }
	ServerResult(ServerResult&&) =default;
	ServerResult& operator=(ServerResult&&) =default;
	ServerResult(ServerResult const&) =default;
	ServerResult& operator=(ServerResult const&) =default;

	static
	ServerResult reupload(std::unique_ptr<Plugin::ServerReuploadIf> reupload) {
		auto ret = ServerResult();
		ret.reupload_ptr = std::move(reupload);
		return ret;
	}
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

	ServerReuploadIf* is_reupload() const {
		return reupload_ptr.get();
	}
	bool is_success() const {
		return !reupload_ptr && succeeded;
	}
	bool is_failure() const {
		return !reupload_ptr && !succeeded;
	}
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP */

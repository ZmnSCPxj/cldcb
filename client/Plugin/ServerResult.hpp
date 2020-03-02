#ifndef CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP
#define CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP

#include<memory>
#include"Plugin/ServerReuploadIf.hpp"

namespace Plugin {

/* The result of sending an incremental change to the
 * server.
 * This can be either:
 * * An increment result, the server wants the plugin to
 *   upload just the incremental change.
 * * A reupload result, the server wants the plugin to
 *   upload the complete file prior to the most recent
 *   change, and *then* the current incremental change.
 * * A failure result, the server is unable to back up.
 */
class ServerResult {
private:
	std::shared_ptr<Plugin::ServerReuploadIf> reupload_ptr;
	std::shared_ptr<Plugin::ServerIncrementIf> increment_ptr;
public:
	ServerResult() : reupload_ptr(nullptr), increment_ptr(nullptr) { }
	ServerResult(ServerResult&&) =default;
	ServerResult& operator=(ServerResult&&) =default;
	ServerResult(ServerResult const&) =default;
	ServerResult& operator=(ServerResult const&) =default;

	static
	ServerResult increment(std::unique_ptr<Plugin::ServerIncrementIf> increment) {
		auto ret = ServerResult();
		ret.increment_ptr = std::move(increment);
		return ret;
	}
	static
	ServerResult reupload(std::unique_ptr<Plugin::ServerReuploadIf> reupload) {
		auto ret = ServerResult();
		ret.reupload_ptr = std::move(reupload);
		return ret;
	}
	static
	ServerResult failure() {
		return ServerResult();
	}

	ServerIncrementIf* is_increment() const {
		return increment_ptr.get();
	}
	ServerReuploadIf* is_reupload() const {
		return reupload_ptr.get();
	}
	bool is_failure() const {
		return !is_increment() && !is_reupload();
	}
};

}

#endif /* CLDCB_CLIENT_PLUGIN_SERVERRESULT_HPP */

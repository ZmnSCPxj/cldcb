#include<exception>
#include<sstream>
#include<vector>
#include"Jsmn/Object.hpp"
#include"Jsmn/jsonify_string.hpp"
#include"LD/Writer.hpp"
#include"Plugin/DbWriteHandler.hpp"
#include"Plugin/MainLoop.hpp"
#include"Util/Logger.hpp"

namespace {

void send_error( LD::Writer& writer, Jsmn::Object const& id
	       , int code, std::string message
	       ) {
	auto os = std::ostringstream();
	os << "{ \"jsonrpc\": \"2.0\""
	   << ", \"id\": " << id
	   << ", \"error\" : { \"code\": " << std::dec << ((int) code)
	   << "              , \"message\": " << Jsmn::jsonify_string(message)
	   << "              }"
	   << "}"
	   ;
	writer.write(os.str());
}

bool validate_db_write_params( std::vector<std::string>& writes
			     , std::uint32_t& data_version
			     , Jsmn::Object const& request
			     ) {
	if (!request.is_object() || !request.has("params"))
		return false;
	auto params = request["params"];
	if (!params.is_object())
		return false;

	auto o_data_version = params["data_version"];
	if (!o_data_version.is_number())
		return false;
	auto n_data_version = (std::uint32_t)(double)o_data_version;

	auto o_writes = params["writes"];
	if (!o_writes.is_array())
		return false;
	auto n_writes = std::vector<std::string>();
	for (auto i = 0; i < o_writes.size(); ++i) {
		auto entry = o_writes[i];
		if (!entry.is_string())
			return false;
		n_writes.push_back((std::string) entry);
	}

	std::swap(writes, n_writes);
	data_version = n_data_version;
	return true;
}

}

namespace Plugin {

int MainLoop::run() {

	for (;;) {
		auto o = Jsmn::Object();

		if (stdin.eof())
			return 0;
		if (!stdin)
			return 1;

		stdin >> o;

		/* If there was whitespace prior to end-of-file.  */
		if (o.is_null() && stdin.eof())
			return 0;

		if (!o.is_object()) {
			log.unusual("Expecting an object in the input stream.");
			continue;
		}
		if (!o.has("id")) {
			log.unusual("Unexpected notification.");
			continue;
		}
		auto id = o["id"];

		if (!o.has("method")) {
			log.unusual("Request object has no method?");
			send_error(writer, id, -32601, "No method to find.");
			continue;
		}
		auto method = o["method"];
		if (!method.is_string()) {
			log.unusual("Request object has non-string method?");
			send_error(writer, id, -32601, "Non-string method.");
			continue;
		}

		/* Dispatch.  */
		auto smethod = std::string(method);
		auto os = std::ostringstream();
		if (smethod == "db_write") {
			std::vector<std::string> writes;
			std::uint32_t data_version;
			if (!validate_db_write_params(writes, data_version, o)) {
				log.unusual("Invalid db_write request");
				send_error(writer, id, -32602, "Invalid parameters.");
				continue;
			}

			bool result;
			try {
				result = handler.handle(data_version, writes);
			} catch (std::exception e) {
				log.BROKEN("Exception: %s", e.what());
				result = false;
			} catch (...) {
				log.BROKEN("Unknown exception.");
				result = false;
			}

			if (result) {
				os << "{ \"jsonrpc\": \"2.0\""
				   << ", \"result\":{\"result\":\"continue\"}"
				   << ", \"id\": " << id
				   << "}"
				   ;
			} else {
				os << "{ \"jsonrpc\": \"2.0\""
				   << ", \"result\":{\"epic\": \"fail\"}"
				   << ", \"id\": " << id
				   << "}"
				   ;
			}

		} else if (smethod == "getmanifest") {
			log.debug("cldcb-plugin manifests!");
			os << "{ \"jsonrpc\": \"2.0\""
			   << ", \"result\": { \"hooks\" : [\"db_write\"]"
			   << "              , \"dynamic\": false"
			   << "              }"
			   << ", \"id\": " << id
			   << "}"
			   ;
		} else if (smethod == "init") {
			/* Do nothing.  */
			log.debug("cldcb-plugin already initialized.");
			os << "{ \"jsonrpc\": \"2.0\""
			   << ", \"result\": { \"result\" : \"continue\" }"
			   << ", \"id\": " << id
			   << "}"
			   ;
		} else {
			/* LOLWHUT?  */
			log.unusual("Unknown method '%s'.", smethod.c_str());
			send_error(writer, id, -32601, "Unknown method.");
			continue;
		}

		writer.write(os.str());
	}
}

}

#include<sstream>
#include"Jsmn/Object.hpp"
#include"Jsmn/jsonify_string.hpp"
#include"LD/Writer.hpp"
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
			/* TODO.  */
			os << "{ \"jsonrpc\": \"2.0\""
			   << ", \"result\": { \"result\" : \"continue\" }"
			   << ", \"id\": " << id
			   << "}"
			   ;
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

#include<assert.h>
#include<sstream>
#include"Jsmn/Object.hpp"
#include"LD/Logger.hpp"
#include"LD/Writer.hpp"

int main() {
	std::ostringstream os;

	/* Bound the lifetimes, to ensure that we wait for
	 * the LD::Writer background thread finishes and writes
	 * everything to the output stream.
	 */
	{
		LD::Writer wr(os);
		LD::Logger log(wr);

		log.debug("debug-level log");
		log.info("info-level log");
		log.unusual("warning-level log");
		log.BROKEN("error-level log");
	}

	std::istringstream is(os.str());
	Jsmn::Object o;

	is >> o;
	assert(o.is_object());
	assert((std::string)o["jsonrpc"] == "2.0");
	assert((std::string)o["method"] == "log");
	assert(o["params"].is_object());
	assert((std::string)o["params"]["level"] == "debug");
	assert((std::string)o["params"]["message"] == "debug-level log");

	is >> o;
	assert(o.is_object());
	assert((std::string)o["jsonrpc"] == "2.0");
	assert((std::string)o["method"] == "log");
	assert(o["params"].is_object());
	assert((std::string)o["params"]["level"] == "info");
	assert((std::string)o["params"]["message"] == "info-level log");

	is >> o;
	assert(o.is_object());
	assert((std::string)o["jsonrpc"] == "2.0");
	assert((std::string)o["method"] == "log");
	assert(o["params"].is_object());
	assert((std::string)o["params"]["level"] == "warn");
	assert((std::string)o["params"]["message"] == "warning-level log");

	is >> o;
	assert(o.is_object());
	assert((std::string)o["jsonrpc"] == "2.0");
	assert((std::string)o["method"] == "log");
	assert(o["params"].is_object());
	assert((std::string)o["params"]["level"] == "error");
	assert((std::string)o["params"]["message"] == "error-level log");

	return 0;
}

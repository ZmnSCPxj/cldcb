#include<assert.h>
#include<sstream>
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include"Jsmn/jsonify_string.hpp"

int main() {
	Jsmn::Parser parser;

	auto jsons = parser.feed("  {} null\ttrue\r\nfalse { \"greeting\" : [ ");
	assert(jsons.size() == 4); /* {} null true false */
	assert(jsons[0].is_object());
	assert(jsons[0].size() == 0);
	assert(jsons[0].keys().size() == 0);
	assert(jsons[0]["nonexistentkey"].is_null());
	assert(jsons[1].is_null());
	assert(!jsons[1]);
	assert(jsons[2].is_boolean());
	assert(jsons[2]);
	assert(jsons[3].is_boolean());
	assert(!jsons[3]);

	jsons = parser.feed(" \"hello\", \"world\", 42 ] }");
	assert(jsons.size() == 1);
	assert(jsons[0].is_object());
	assert(jsons[0].size() == 1);
	assert(jsons[0].keys().size() == 1);
	assert(jsons[0].keys()[0] == "greeting");
	assert(jsons[0].has("greeting"));
	auto greeting = jsons[0]["greeting"];
	assert(greeting.is_array());
	assert(!greeting.is_null());
	assert(!greeting.is_object());
	assert(!greeting.is_boolean());
	assert(!greeting.is_number());
	assert(!greeting.is_string());
	assert(greeting[0].is_string());
	assert((std::string)greeting[0] == "hello");
	assert(greeting[1].is_string());
	assert((std::string)greeting[1] == "world");
	assert(greeting[2].is_number());
	assert(!greeting[2].is_null());
	assert(!greeting[2].is_boolean());
	assert(!greeting[2].is_array());
	assert(!greeting[2].is_string());
	assert(!greeting[2].is_object());
	assert((double)greeting[2] == 42);

	auto is = std::istringstream("[1,2] \"\\u0000\"  \"\\t\\r\\n\\f\\b\\\"\\\\\"");
	Jsmn::Object o;
	is >> o;
	assert(o.is_array());
	assert(o.size() == 2);
	assert((double)o[0] == 1);
	assert((double)o[1] == 2);
	is >> o;
	assert(o.is_string());
	assert(!o.is_array());
	assert(!o.is_object());
	assert(!o.is_boolean());
	assert(!o.is_null());
	assert(!o.is_number());
	assert((std::string)o == std::string("\0", 1));
	is >> o;
	assert((std::string)o == "\t\r\n\f\b\"\\");

	auto orig_string = std::string("\"\\\t\r\n\f\b\v");
	auto is2 = std::istringstream(Jsmn::jsonify_string(orig_string));
	is2 >> o;
	assert((std::string)o == orig_string);

	return 0;
}

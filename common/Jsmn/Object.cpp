#include<assert.h>
#include<sstream>
#include"Jsmn/Detail/EndAdvancer.hpp"
#include"Jsmn/Detail/ParseResult.hpp"
#include"Jsmn/Detail/Str.hpp"
#include"Jsmn/Detail/Token.hpp"
#include"Jsmn/Detail/Type.hpp"
#include"Jsmn/Object.hpp"
#include"Jsmn/Parser.hpp"
#include"Util/Str.hpp"

namespace Jsmn {

class Object::Impl {
private:
	std::shared_ptr<Detail::ParseResult> parse_result;
	unsigned int i;

	Detail::Token& token() {
		return parse_result->tokens[i];
	}
	Detail::Token const& token() const {
		return parse_result->tokens[i];
	}
	char const& at(size_t i) const {
		return parse_result->orig_string[i];
	}
	std::string at(size_t b, size_t e) const {
		auto sb = parse_result->orig_string.cbegin();
		return std::string(sb + b, sb + e);
	}

public:
	Impl( std::shared_ptr<Detail::ParseResult> parse_result_
	    , unsigned int i_
	    ) : parse_result(std::move(parse_result_))
	      , i(i_)
	      { }

	Detail::Type type() const {
		return token().type;
	}
	char first_char() const {
		return at(token().start);
	}

	explicit operator bool() const {
		auto& tok = token();
		if (tok.type != Detail::Primitive)
			throw TypeError();

		auto c = at(tok.start);
		if (c == 'n' || c == 'f')
			return false;
		if (c == 't')
			return true;

		/* Other primitive.  */
		throw TypeError();
	}

	explicit operator std::string() const {
		auto& tok = token();
		if (tok.type != Detail::String)
			throw TypeError();
		return Detail::Str::from_escaped(at(tok.start, tok.end));
	}

	explicit operator double() const {
		auto& tok = token();
		if (tok.type != Detail::Primitive)
			throw TypeError();

		auto c = at(tok.start);
		if (c == 'n' || c == 'f' || c == 't')
			throw TypeError();

		return Detail::Str::to_double(at(tok.start, tok.end));
	}

	/* Object/Array.  */
	std::size_t size() const {
		auto& tok = token();
		switch (tok.type) {
		case Detail::Array:
		case Detail::Object:
			break;
		default:
			throw TypeError();
		}

		return tok.size;
	}

	/* Object.  */
	std::vector<std::string> keys() const {
		auto& tok = token();
		if (tok.type != Detail::Object)
			throw TypeError();

		auto ret = std::vector<std::string>();

		auto tokptr = &tok + 1;
		for (auto i = 0; i < tok.size; ++i, ++tokptr, Detail::Token::next(tokptr)) {
			auto ekey = at(tokptr->start, tokptr->end);
			auto key = Detail::Str::from_escaped(ekey);
			ret.push_back(key);
		}

		return ret;
	}
	bool has(std::string const& s) const {
		auto& tok = token();
		if (tok.type != Detail::Object)
			throw TypeError();

		auto tokptr = &tok + 1;
		for (auto i = 0; i < tok.size; ++i, ++tokptr, Detail::Token::next(tokptr)) {
			auto ekey = at(tokptr->start, tokptr->end);
			auto key = Detail::Str::from_escaped(ekey);
			if (key == s)
				return true;
		}
		return false;
	}
	std::shared_ptr<Impl> operator[](std::string const& s) const {
		auto my_i = i;
		auto& tok = token();
		if (tok.type != Detail::Object)
			throw TypeError();

		auto tokptr = &tok + 1;
		for (auto i = 0; i < tok.size; ++i, ++tokptr, Detail::Token::next(tokptr)) {
			auto ekey = at(tokptr->start, tokptr->end);
			auto key = Detail::Str::from_escaped(ekey);
			if (key == s) {
				++tokptr;
				return std::make_shared<Impl>( parse_result
							     , tokptr - &parse_result->tokens[0]
							     );
			}
		}
		return nullptr;
	}

	/* Array.  */
	std::shared_ptr<Impl> operator[](std::size_t i) const {
		auto my_i = this->i;
		auto& tok = token();
		if (tok.type != Detail::Array)
			throw TypeError();

		if (i >= tok.size)
			return nullptr;

		auto tokptr = &tok + 1;
		for (auto step = 0; step < i; ++step)
			Detail::Token::next(tokptr);

		return std::make_shared<Impl>( parse_result
					     , tokptr - &parse_result->tokens[0]
					     );
	}
};

Object::Object() : pimpl(nullptr) { }

Object::Object( std::shared_ptr<Detail::ParseResult> parse_result
	      , unsigned int i
	      ) : pimpl(std::make_shared<Impl>(std::move(parse_result), i)) {}

bool Object::is_null() const {
	if (!pimpl)
		return true;
	return pimpl->type() == Detail::Primitive && pimpl->first_char() == 'n';
}
bool Object::is_boolean() const {
	if (!pimpl)
		return false;
	auto c = pimpl->first_char();
	return pimpl->type() == Detail::Primitive && ((c == 'f') || (c == 't'));
}
bool Object::is_string() const {
	if (!pimpl)
		return false;
	return pimpl->type() == Detail::String;
}
bool Object::is_object() const {
	/* null is not considered an object, unlike JavaScript.  */
	if (!pimpl)
		return false;
	return pimpl->type() == Detail::Object;
}
bool Object::is_array() const {
	if (!pimpl)
		return false;
	return pimpl->type() == Detail::Array;
}
bool Object::is_number() const {
	if (!pimpl)
		return false;
	auto c = pimpl->first_char();
	return pimpl->type() == Detail::Primitive
	    && ((c != 't') && (c != 'f') && (c != 'n'))
	     ;
}

Object::operator bool() const {
	if (!pimpl)
		return false;
	return (bool) (*pimpl);
}
Object::operator std::string() const {
	if (!pimpl)
		throw TypeError();
	return (std::string) (*pimpl);
}
Object::operator double() const {
	if (!pimpl)
		throw TypeError();
	return (double) (*pimpl);
}

std::size_t Object::size() const {
	if (!pimpl)
		throw TypeError();
	return pimpl->size();
}
std::vector<std::string> Object::keys() const {
	if (!pimpl)
		throw TypeError();
	return pimpl->keys();
}
bool Object::has(std::string const& s) const {
	if (!pimpl)
		throw TypeError();
	return pimpl->has(s);
}
Object Object::operator[](std::string const& s) const {
	if (!pimpl)
		throw TypeError();
	auto ret = Object();
	ret.pimpl = (*pimpl)[s];
	return ret;
}

Object Object::operator[](std::size_t i) const {
	if (!pimpl)
		throw TypeError();
	auto ret = Object();
	ret.pimpl = (*pimpl)[i];
	return ret;
}

/* Implements indented printing.  */
namespace {

void print_indent(std::ostream& os, std::size_t indent) {
	for (auto i = std::size_t(0); i < indent; ++i)
		os << "\t";
}
void print( std::ostream& os
	  , std::size_t indent
	  , Jsmn::Object const& o
	  ) {
	if (o.is_null()) {
		os << "null";
	} else if (o.is_boolean()) {
		os << (o ? "true" : "false");
	} else if (o.is_string()) {
		os << '"' << Detail::Str::to_escaped((std::string) o) << '"';
	} else if (o.is_object()) {
		auto keys = o.keys();
		if (keys.size() == 0) {
			os << "{ }";
		} else {
			os << "{" << std::endl;
			for (auto i = 0; i < keys.size(); ++i) {
				print_indent(os, indent + 1);

				auto& key = keys[i];
				os << '"' << Detail::Str::to_escaped(key)
				   << '"' << " : "
				   ;
				auto value = o[key];
				print(os, indent + 1, value);
				if (i != keys.size() - 1)
					os << ',';
				os << std::endl;
			}
			print_indent(os, indent);
			os << "}";
		}
	} else if (o.is_array()) {
		if (o.size() == 0) {
			os << "[ ]";
		} else {
			os << '[' << std::endl;
			for (auto i = 0; i < o.size(); ++i) {
				print_indent(os, indent + 1);

				auto value = o[i];
				print(os, indent + 1, value);
				if (i != o.size() - 1)
					os << ',';
				os << std::endl;
			}
			print_indent(os, indent);
			os << ']';
		}
	} else if (o.is_number()) {
		os << Detail::Str::from_double((double) o);
	} else {
		/* Impossible. */
		assert(0 == 1);
	}
}

}

std::ostream& operator<<(std::ostream& os, Jsmn::Object const& o) {
	print(os, 0, o);
	return os;
}

namespace {

/* Reads characters from an input stream, saving them in a buffer that
 * will later be fed into a Jsmn::Parser.
 */
class StreamSourceReader : public Detail::SourceReader {
private:
	std::istream& is;
	std::string buffer;
public:
	explicit
	StreamSourceReader(std::istream& is_)
		: is(is_), buffer("") { }

	std::pair<bool, char> read() {
		auto c = char('0');
		if (!is || is.eof())
			return std::make_pair(false, '0');
		is.read(&c, 1);
		buffer.push_back(c);
		return std::make_pair(true, c);
	}

	std::string get_buffer() {
		auto ret = std::move(buffer);
		buffer = "";
		return ret;
	}
};

}

std::istream& operator>>(std::istream& is, Jsmn::Object& o) {
	auto started = false;

	StreamSourceReader ssr(is);
	Detail::EndAdvancer ender(ssr);
	Jsmn::Parser parser;

	is >> std::ws;

	for(;;) {
		if (!is || is.eof()) {
			if (!started)
				return is;
			throw std::runtime_error("Unexpected end-of-file.");
		}
		started = true;
		ender.scan();

		auto ret = parser.feed(ssr.get_buffer());
		assert(ret.size() < 2);

		if (ret.size() != 0) {
			o = std::move(ret[0]);
			return is;
		}

	}
}

}


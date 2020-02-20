#ifndef CLDCB_COMMON_S_HPP
#define CLDCB_COMMON_S_HPP

#include<cstdint>
#include<queue>
#include<stdexcept>
#include<string>
#include<vector>

namespace S {

/* Thrown when deserialization fails due to EOF.  */
/* FIXME: backtrace-capturing.  */
class DeserializationTruncated : public std::runtime_error {
public:
	DeserializationTruncated()
		: std::runtime_error("Deserialization truncated due to end-of-file")
		{ }
};
/* Thrown when deserialization fails due to an invalid byte in the
 * incoming byte stream.
 */
/* FIXME: backtrace-capturing.  */
class InvalidByte : public std::runtime_error {
public:
	InvalidByte()
		: std::runtime_error("Invalid byte in data for deserialization")
		{ }
};

/*--------------------------------------------------------------------------*/

template<typename A>
void put_byte(A& a, std::uint8_t);

template<>
void put_byte<std::vector<std::uint8_t>>( std::vector<std::uint8_t>& a
					, std::uint8_t b
					) {
	a.push_back(b);
}
template<>
void put_byte<std::queue<std::uint8_t>>( std::queue<std::uint8_t>& a
				       , std::uint8_t b
				       ) {
	a.push(b);
}

template<typename A>
void serialize(A& a, bool const& t) {
	put_byte(a, t ? 1 : 0);
}
template<typename A>
void serialize(A& a, std::uint8_t const& t) {
	put_byte(a, t);
}
template<typename A>
void serialize(A& a, std::int8_t const& t) {
	serialize(a, std::uint8_t(t));
}
template<typename A>
void serialize(A& a, std::uint16_t const& t) {
	auto d0 = std::uint8_t((t >> 8) & 0xff);
	auto d1 = std::uint8_t((t >> 0) & 0xff);

	put_byte(a, d0);
	put_byte(a, d1);
}
template<typename A>
void serialize(A& a, std::int16_t const& t) {
	serialize(a, std::uint16_t(t));
}
template<typename A>
void serialize(A& a, std::uint32_t const& t) {
	auto d0 = std::uint8_t((t >> 24) & 0xff);
	auto d1 = std::uint8_t((t >> 16) & 0xff);
	auto d2 = std::uint8_t((t >>  8) & 0xff);
	auto d3 = std::uint8_t((t >>  0) & 0xff);

	put_byte(a, d0);
	put_byte(a, d1);
	put_byte(a, d2);
	put_byte(a, d3);
}
template<typename A>
void serialize(A& a, std::int32_t const& t) {
	serialize(a, std::uint32_t(t));
}
template<typename A>
void serialize(A& a, std::uint64_t const& t) {
	auto d0 = std::uint8_t((t >> 56) & 0xff);
	auto d1 = std::uint8_t((t >> 48) & 0xff);
	auto d2 = std::uint8_t((t >> 40) & 0xff);
	auto d3 = std::uint8_t((t >> 32) & 0xff);
	auto d4 = std::uint8_t((t >> 24) & 0xff);
	auto d5 = std::uint8_t((t >> 16) & 0xff);
	auto d6 = std::uint8_t((t >>  8) & 0xff);
	auto d7 = std::uint8_t((t >>  0) & 0xff);

	put_byte(a, d0);
	put_byte(a, d1);
	put_byte(a, d2);
	put_byte(a, d3);
	put_byte(a, d4);
	put_byte(a, d5);
	put_byte(a, d6);
	put_byte(a, d7);
}
template<typename A>
void serialize(A& a, std::int64_t const& t) {
	serialize(a, std::uint64_t(t));
}

template<typename A>
void serialize(A& a, std::string const& s) {
	auto len = std::uint64_t(s.length());
	serialize(a, len);
	for (auto i = std::uint64_t(0); i < len; ++i) {
		std::uint8_t b = s[i];
		serialize(a, b);
	}
}

template<typename A, typename T>
void serialize(A& a, std::vector<T> const& t) {
	auto len = std::uint64_t(t.size());
	serialize(a, len);
	for (auto i = std::uint64_t(0); i < len; ++i) {
		serialize(a, t[i]);
	}
}

/*--------------------------------------------------------------------------*/

template<typename A>
std::uint8_t get_byte(A&);

template<>
std::uint8_t get_byte<std::queue<std::uint8_t>>(std::queue<std::uint8_t>& a) {
	if (a.empty())
		throw DeserializationTruncated();
	auto ret = a.front();
	a.pop();
	return ret;
}

template<typename A>
void deserialize(A& a, bool& t) {
	auto b = get_byte(a);
	switch (b) {
	case 0: t = false; return;
	case 1: t = true; return;
	default:
		throw InvalidByte();
	}
}
template<typename A>
void deserialize(A& a, std::uint8_t& t) {
	t = get_byte(a);
}
template<typename A>
void deserialize(A& a, std::int8_t& t) {
	std::uint8_t b;
	deserialize(a, b);
	t = (std::int8_t) b;
}
template<typename A>
void deserialize(A& a, std::uint16_t& t) {
	auto d0 = get_byte(a);
	auto d1 = get_byte(a);
	t = (std::uint16_t(d0) << 8)
	  + (std::uint16_t(d1) << 0)
	  ;
}
template<typename A>
void deserialize(A& a, std::int16_t& t) {
	std::uint16_t b;
	deserialize(a, b);
	t = (std::int16_t) b;
}
template<typename A>
void deserialize(A& a, std::uint32_t& t) {
	auto d0 = get_byte(a);
	auto d1 = get_byte(a);
	auto d2 = get_byte(a);
	auto d3 = get_byte(a);
	t = (std::uint32_t(d0) << 24)
	  + (std::uint32_t(d1) << 16)
	  + (std::uint32_t(d2) <<  8)
	  + (std::uint32_t(d3) <<  0)
	  ;
}
template<typename A>
void deserialize(A& a, std::int32_t& t) {
	std::uint32_t b;
	deserialize(a, b);
	t = (std::int32_t) b;
}
template<typename A>
void deserialize(A& a, std::uint64_t& t) {
	auto d0 = get_byte(a);
	auto d1 = get_byte(a);
	auto d2 = get_byte(a);
	auto d3 = get_byte(a);
	auto d4 = get_byte(a);
	auto d5 = get_byte(a);
	auto d6 = get_byte(a);
	auto d7 = get_byte(a);
	t = (std::uint64_t(d0) << 56)
	  + (std::uint64_t(d1) << 48)
	  + (std::uint64_t(d2) << 40)
	  + (std::uint64_t(d3) << 32)
	  + (std::uint64_t(d4) << 24)
	  + (std::uint64_t(d5) << 16)
	  + (std::uint64_t(d6) <<  8)
	  + (std::uint64_t(d7) <<  0)
	  ;
}
template<typename A>
void deserialize(A& a, std::int64_t& t) {
	std::uint64_t b;
	deserialize(a, b);
	t = (std::int64_t) b;
}

template<typename A>
void deserialize(A& a, std::string& t) {
	std::uint64_t len;
	deserialize(a, len);
	std::string new_t;
	for (auto i = std::uint64_t(0); i < len; ++i) {
		std::uint8_t b;
		deserialize(a, b);
		new_t.push_back(b);
	}
	t = std::move(new_t);
}

template<typename A, typename T>
void deserialize(A& a, std::vector<T>& t) {
	std::uint64_t len;
	deserialize(a, len);
	std::vector<T> new_t;
	for (auto i = std::uint64_t(0); i < len; ++i) {
		T b;
		deserialize(a, b);
		new_t.emplace_back(std::move(b));
	}
	t = std::move(new_t);
}

}

#endif /* CLDCB_COMMON_S_HPP */

#ifndef CLDCB_COMMON_PROTOCOL_MESSAGE_HPP
#define CLDCB_COMMON_PROTOCOL_MESSAGE_HPP

#include<assert.h>
#include<cstdint>
#include<map>
#include"S.hpp"

namespace Protocol {

/* A generic message structure.  */
struct Message {
	/* Message identifier.  */
	std::uint16_t id;
	/* Message TLVs.  */
	std::map<std::uint8_t, std::vector<std::uint8_t>> tlvs;

	Message() : id(0), tlvs() { }
	Message(Message const&) =default;
	Message(Message&&) =default;
	Message& operator=(Message const&) =default;
	Message& operator=(Message&&) =default;
};

}

namespace S {

/* Note: Our protocol requires < 65536 bytes per complete message,
 * there is **no** checking done here that this is the case.
 */
template<typename A>
void serialize(A& a, ::Protocol::Message const& t) {
	serialize(a, t.id);
	auto is_terminated = false;
	for (auto& tlv : t.tlvs) {
		serialize(a, tlv.first);
		assert(tlv.second.size() <= 65535);
		serialize(a, std::uint16_t(tlv.second.size()));
		for (auto& byte : tlv.second)
			serialize(a, byte);
		if (tlv.first == 0xFF)
			is_terminated = true;
	}
	if (!is_terminated) {
		serialize(a, std::uint8_t(0xFF));
		serialize(a, std::uint16_t(0));
	}
}
template<typename A>
void deserialize(A& a, ::Protocol::Message& t) {
	deserialize(a, t.id);
	t.tlvs.clear();
	for (;;) {
		auto type = std::uint8_t();
		deserialize(a, type);
		auto len = std::uint16_t();
		deserialize(a, len);
		auto value = std::vector<std::uint8_t>(std::size_t(len));
		for (auto i = std::uint16_t(0); i < l; ++i)
			deserialize(a, value[0]);
		t.tlvs.insert(std::make_pair(type, std::move(value)));
		if (type == 0xFF)
			break;
	}
}

}

#endif /* CLDCB_COMMON_PROTOCOL_MESSAGE_HPP */

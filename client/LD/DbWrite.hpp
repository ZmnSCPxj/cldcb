#ifndef CLDCB_CLIENT_LD_DBWRITE_HPP
#define CLDCB_CLIENT_LD_DBWRITE_HPP

#include<cstdint>
#include<string>
#include<vector>
#include"S.hpp"

namespace LD {

/* Represents the data from a `db_write` hook.  */
class DbWrite {
public:
	std::uint32_t data_version;
	std::vector<std::string> writes;
};

}

namespace S {

template<typename A>
void serialize(A& a, ::LD::DbWrite const& t) {
	/* Version byte.
	 * TODO
	 * In the future we might want to try
	 * compressing the writes, as they are
	 * text SQL statements.
	 * For now, version 0 is uncompressed
	 * SQL statements.
	 * For our use-case, we want a fast
	 * compressor, and can sacrifice a
	 * slow decompressor, since we will
	 * be compressing many db writes but
	 * will only rarely recover.
	 */
	auto version = std::uint8_t(0);
	serialize(a, version);
	serialize(a, t.data_version);
	serialize(a, t.writes);
}
template<typename A>
void deserialize(A& a, ::LD::DbWrite& t) {
	/* Version byte.  */
	auto version = std::uint8_t(0);
	deserialize(a, version);
	if (version != 0)
		throw InvalidByte();
	deserialize(a, t.data_version);
	deserialize(a, t.writes);
}

}

#endif /* CLDCB_CLIENT_LD_DBWRITE_HPP */

#ifndef CLDCB_COMMON_STREAM_SOURCE_HPP
#define CLDCB_COMMON_STREAM_SOURCE_HPP

#include<cstdint>
#include<cstdlib>
#include<vector>

namespace Stream {

class Source {
public:
	virtual ~Source() { }

	/* Return empty or short vector if at EOF.  */
	virtual
	std::vector<std::uint8_t> read(std::size_t size) =0;
};

}

#endif /* CLDCB_COMMON_STREAM_SOURCE_HPP */

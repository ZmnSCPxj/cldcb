#ifndef CLDCB_COMMON_STREAM_SINK_HPP
#define CLDCB_COMMON_STREAM_SINK_HPP

#include<cstdint>
#include<vector>

namespace Stream {

class Sink {
public:
	virtual ~Sink() { }

	virtual
	void write(std::vector<std::uint8_t> const&) =0;
};

}

#endif /* CLDCB_COMMON_STREAM_SINK_HPP */

#ifndef CLDCB_COMMON_STREAM_SINKSOURCE_HPP
#define CLDCB_COMMON_STREAM_SINKSOURCE_HPP

#include"Stream/Sink.hpp"
#include"Stream/Source.hpp"

namespace Stream {

class SinkSource : public Sink, public Source { };

}

#endif /* CLDCB_COMMON_STREAM_SINKSOURCE_HPP */

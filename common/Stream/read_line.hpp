#ifndef CLDCB_COMMON_STREAM_READ_LINE_HPP
#define CLDCB_COMMON_STREAM_READ_LINE_HPP

#include<string>
#include<istream>

namespace Stream {

/* Reads a line of text and returns it.
 * Lines are terminated by \n only.
 */
std::string read_line(std::istream&);

}

#endif /* CLDCB_COMMON_STREAM_READ_LINE_HPP */

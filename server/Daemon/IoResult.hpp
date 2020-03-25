#ifndef CLDCB_SERVER_DAEMON_IORESULT_HPP
#define CLDCB_SERVER_DAEMON_IORESULT_HPP

#include<cstdint>
#include<vector>

namespace Daemon {

enum IoResultType
/* The IO completed.
 * For reads, data is exactly the size requested.
 * For writes, data is empty.
 */
{ IoOk
/* An end-of-file condition occurred.
 * Partial data may be available.
 * For reads, data contains what data was
 * read before end-of-file.
 * For writes, data contains the data that
 * was not written when it was disconnected.
 */
, IoEof
/* The IO timed out.
 * Partial data may be available.
 */
, IoTimeout
/* A SIGINT or SIGTERM was received, or
 * some other system-level error occurred.
 * Data might exist, but would probably
 * not matter since we should exit soon.
 */
, IoBroken
};

class IoResult {
public:
	IoResultType result;
	std::vector<std::uint8_t> data;
};

}

#endif /* CLDCB_SERVER_DAEMON_IORESULT_HPP */

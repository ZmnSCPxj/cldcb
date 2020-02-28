#ifndef CLDCB_SERVER_EV_WAIT_READABLE_HPP
#define CLDCB_SERVER_EV_WAIT_READABLE_HPP

#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/* Wait for the specified fd to become readable.  */
Ev::Io<int> wait_readable(int fd);

/* Wait for one of the specified fds to become readable.
 * Returns the fd which became readable.
 */
Ev::Io<int> wait_readable(std::vector<int>);

/* Wait for one of the specified fds to become readable.
 * Terminate the argument list with a -1.
 * Returns the fd which became readable.
 */
Ev::Io<int> wait_readables(int fd0, ...);

}

#endif /* CLDCB_SERVER_EV_WAIT_READABLE_HPP */

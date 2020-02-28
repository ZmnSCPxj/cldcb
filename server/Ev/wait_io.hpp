#ifndef CLDCB_SERVER_EV_WAIT_IO_HPP
#define CLDCB_SERVER_EV_WAIT_IO_HPP

#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Ev {

enum WaitDirection
{ WaitRead
, WaitWrite
};

/* Wait for one of the file descriptors to be
 * ready for read or write.
 * Return the fd that was ready first.
 */
Ev::Io<int> wait_io(std::vector<std::pair<int, WaitDirection>>);
/* As above, but with the calling convention:
 * wait_io( fd, Ev::WaitRead
 *        , fd2, Ev::WaitWrite
 *        , fd3, Ev::WaitRead
 *        , -1
 *        );
 */
Ev::Io<int> wait_io(int fd0, WaitDirection dir0, ...);

}

#endif /* CLDCB_SERVER_EV_WAIT_IO_HPP */

#ifndef CLDCB_SERVER_EV_WAIT_IO_HPP
#define CLDCB_SERVER_EV_WAIT_IO_HPP

#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Ev {

enum WaitDirection : int
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
Ev::Io<int> wait_io(int fd0, ...);

/* Wait for one of the file descriptors to be
 * ready for read or write, or for a timeout
 * to occur.
 * Timeout is in units of seconds.
 * If timeout is negative, no timeout (i.e.
 * equivalent to wait_io).
 * Return -1 if timed out, else the fd that
 * was ready first.
 */
Ev::Io<int> wait_io_until( std::vector<std::pair<int, WaitDirection>>
			 , double timeout
			 );
/* As above, but with the calling convention:
 * wait_io_until( fd, Ev::WaitRead
 *              , fd, Ev::WaitWrite
 *              , fd, Ev::WaitRead
 *              , -1
 *              , double(1)
 *              );
 */
Ev::Io<int> wait_io_until(int fd0, ...);

}

#endif /* CLDCB_SERVER_EV_WAIT_IO_HPP */

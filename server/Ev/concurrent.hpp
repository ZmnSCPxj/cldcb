#ifndef CLDCB_SERVER_EV_CONCURRENT_HPP
#define CLDCB_SERVER_EV_CONCURRENT_HPP

namespace Ev { template<typename a> class Io; }

namespace Ev {

/* Cause the other Io<int> to execute when we are
 * no longer busy.
 * Note: This will always return a 0, and the return
 * value of the given Io<int> is ignored.
 */
Io<int> concurrent(Io<int> o);

}

#endif /* CLDCB_SERVER_EV_CONCURRENT_HPP */

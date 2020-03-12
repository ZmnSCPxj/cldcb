#ifndef CLDCB_SERVER_EV_YIELD_HPP
#define CLDCB_SERVER_EV_YIELD_HPP

#include<functional>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/* Lets other concurrent tasks run as well.
 * Highly recommended for recursive functions:
 *
 * Ev::Io<Whatever> recursive(int x)  {
 *     // Notice the Ev::yield() here.
 *     return Ev::yield().then([x](int) {
 *         if (x == 0)
 *             return Ev::lift_io(Whatever());
 *         else
 *             return recursive(x - 1);
 *     });
 * }
 */
Io<int> yield();

/* Shared by some other Ev objects.  */
namespace Detail {

void yield_pass(std::function<void()> pass);

}

}

#endif /* CLDCB_SERVER_EV_YIELD_HPP */

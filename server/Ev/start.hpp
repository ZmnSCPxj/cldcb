#ifndef CLDCB_SERVER_EV_START_HPP
#define CLDCB_SERVER_EV_START_HPP

/* Entry point to a libev mainloop.  */

namespace Ev { template<typename a> class Io; }

namespace Ev {

int start(Io<int>);

}

#endif /* CLDCB_SERVER_EV_START_HPP */

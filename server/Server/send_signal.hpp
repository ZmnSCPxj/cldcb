#ifndef CLDCB_SERVER_SERVER_SEND_SIGNAL_HPP
#define CLDCB_SERVER_SERVER_SEND_SIGNAL_HPP

namespace Server {

/* Look for the server PID, then send the specified signal.
 * Return true on success, false otherwise.
 */
bool send_signal(int);

}

#endif /* CLDCB_SERVER_SERVER_SEND_SIGNAL_HPP */

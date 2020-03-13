#ifndef CLDCB_SERVER_BACKUP_PINGPONGMESSENGER_HPP
#define CLDCB_SERVER_BACKUP_PINGPONGMESSENGER_HPP

#include<memory>
#include"Daemon/Messenger.hpp"

namespace Ev { template<typename a> class Io; }
namespace Protocol { class Message; }

namespace Backup {

/* A wrapper around Daemon::Messenger which skips
 * incoming `ping` and `pong` messages.
 */
class PingPongMessenger {
private:
	Daemon::Messenger& messenger;
	bool timedout;

public:
	explicit PingPongMessenger(Daemon::Messenger& messenger_)
		: messenger(messenger_) { }

	/* receive_message times out after 10 seconds.  */
	Ev::Io<std::unique_ptr<Protocol::Message>> receive_message();

	Ev::Io<bool> send_message(Protocol::Message);

	int get_fd() const { return messenger.get_fd(); }
};

}

#endif /* CLDCB_SERVER_BACKUP_PINGPONGMESSENGER_HPP */


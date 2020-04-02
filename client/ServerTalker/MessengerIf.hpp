#ifndef CLDCB_CLIENT_SERVERTALKER_MESSENGERIF_HPP
#define CLDCB_CLIENT_SERVERTALKER_MESSENGERIF_HPP

namespace Protocol { class Message; }

namespace ServerTalker {

/* An object which receives and sends messages over this thread.  */
class MessengerIf {
public:
	virtual ~MessengerIf() {}

	/* Block until we receive a message.  Return nullptr
	 * if no more messages or some other error occurred.
	 */
	virtual
	std::unique_ptr<Protocol::Message> receive_message() =0;
	/* Block until we completely send the given message.
	 * Return false if some error occurred.
	 */
	virtual
	bool send_message(Protocol::Message) =0;

	/* Get the fd behind this messenger.  */
	virtual
	int get_fd() const =0;
};

}

#endif /* CLDCB_CLIENT_SERVERTALKER_MESSENGERIF_HPP */

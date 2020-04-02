#ifndef CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP
#define CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP

#include<memory>
#include<queue>
#include"ServerTalker/MessengerIf.hpp"

namespace ServerTalker { namespace Detail {

/* Automatically handles ping from the server.
 * receive_message() will not return ping, and
 * this object will automatically respond with
 * a pong to the server if the server sends a
 * ping.
 */
class DePinger : public ServerTalker::MessengerIf {
private:
	std::unique_ptr<ServerTalker::MessengerIf> messenger;
	std::queue<std::unique_ptr<Protocol::Message>> queue;

public:
	DePinger() =delete;
	DePinger(DePinger&&) =delete;
	DePinger(DePinger const&) =delete;

	explicit DePinger( std::unique_ptr<ServerTalker::MessengerIf> messenger
			 );
	~DePinger();

	std::unique_ptr<Protocol::Message> receive_message() override;
	bool send_message(Protocol::Message message) override;
	int get_fd() const override { return messenger->get_fd(); }

	/* The using thread should invoke this whenever it notices that
	 * the returned fd is ready for receiving.
	 */
	void deping();
};

}}

#endif /* CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP */

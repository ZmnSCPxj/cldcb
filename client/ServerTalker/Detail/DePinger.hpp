#ifndef CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP
#define CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP

#include<future>
#include<memory>

namespace ServerTalker { class Messenger; }
namespace Protocol { class Message; }
namespace Util { class Logger; }

namespace ServerTalker { namespace Detail {

/* Automatically handles ping from the server.
 * receive_message() will not return ping, and
 * this object will automatically respond with
 * a pong to the server if the server sends a
 * ping.
 */
class DePinger {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;
public:
	DePinger() =delete;
	DePinger(DePinger&&) =delete;
	DePinger(DePinger const&) =delete;

	explicit DePinger( std::unique_ptr<ServerTalker::Messenger> messenger
			 );

	~DePinger();

	std::future<std::unique_ptr<Protocol::Message>>
	receive_message();

	std::future<bool>
	send_message(Protocol::Message message);
};

}}

#endif /* CLDCB_CLIENT_SERVERTALKER_DETAIL_DEPINGER_HPP */

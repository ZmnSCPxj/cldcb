#ifndef CLDCB_CLIENT_SERVERTALKER_MESSENGER_HPP
#define CLDCB_CLIENT_SERVERTALKER_MESSENGER_HPP

#include<memory>
#include<utility>
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"
#include"ServerTalker/MessengerIf.hpp"

namespace Protocol { class Message; }
namespace Util { class Logger; }

namespace ServerTalker {

class Messenger : public ServerTalker::MessengerIf {
private:
	Util::Logger& logger;
	Net::SocketFd fd;
	Noise::Encryptor enc;

public:
	Messenger() =delete;
	Messenger(Messenger&&)=default;

	Messenger( Util::Logger& logger_
		 , Net::SocketFd fd_
		 , Noise::Encryptor enc_
		 ) : logger(logger_)
		   , fd(std::move(fd_))
		   , enc(std::move(enc_))
		   { }

	/* Read a message, or return nullptr if disconnected
	 * or other error detected.
	 */
	std::unique_ptr<Protocol::Message> receive_message();
	/* Write a message, return true if okay, false if
	 * failed.
	 */
	bool send_message(Protocol::Message message);

	int get_fd() const { return fd.get(); }
};

}

#endif /* CLDCB_CLIENT_SERVERTALKER_MESSENGER_HPP */

#ifndef CLDCB_SERVER_DAEMON_MESSENGER_HPP
#define CLDCB_SERVER_DAEMON_MESSENGER_HPP

#include<cstdint>
#include<memory>
#include<vector>
#include"Net/SocketFd.hpp"
#include"Noise/Encryptor.hpp"

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Protocol { class Message; }
namespace Util { class Logger; }

namespace Daemon {

class Messenger {
private:
	Util::Logger& logger;
	Daemon::Breaker& breaker;
	Net::SocketFd fd;
	Noise::Encryptor enc;

public:
	Messenger() =delete;
	Messenger(Messenger&&)=default;

	Messenger( Util::Logger& logger_
		 , Daemon::Breaker& breaker_
		 , Net::SocketFd fd_
		 , Noise::Encryptor enc_
		 ) : logger(logger_)
		   , breaker(breaker_)
		   , fd(std::move(fd_))
		   , enc(std::move(enc_))
		   { }

	/* Wait for a message, or return with nullptr if timed out
	 * or a break condition occurs.
	 * The given initial_timeout is only for timeout of the
	 * message header; once there is any data received at all,
	 * it will block forever until a break condition.
	 *
	 * If the call timed out before receiving any data at all,
	 * the timedout flag is set to true, otherwise it will be
	 * false.
	 * Caller is responsible for keeping the flag alive, obviously,
	 * as it will be saved by reference in the returned Ev::Io.
	 */
public:
	Ev::Io<std::unique_ptr<Protocol::Message>>
	receive_message(double initial_timeout, bool& timedout);

private:
	/* Read length part of message.  */
	Ev::Io<std::unique_ptr<std::uint16_t>>
	initial_receive(double initial_timeout, bool& timedout);
	Ev::Io<std::unique_ptr<std::uint16_t>>
	read_length(std::vector<std::uint8_t>);
	/* Read actual message.  */
	Ev::Io<std::unique_ptr<Protocol::Message>>
	read_message(std::vector<std::uint8_t>, std::size_t);

	/* Send the given message to the client.  */
public:
	Ev::Io<bool>
	send_message(Protocol::Message message);

	/* Get the fd, for log messages.  */
	int get_fd() const { return fd.get(); }
};

}

#endif /* CLDCB_SERVER_DAEMON_MESSENGER_HPP */

#ifndef CLDCB_SERVER_BACKUP_SERVICELOOP_HPP
#define CLDCB_SERVER_BACKUP_SERVICELOOP_HPP

#include<memory>
#include<string>
#include"Daemon/Messenger.hpp"
#include"Secp256k1/PubKey.hpp"

namespace Daemon { class Breaker; }
namespace Ev { template<typename a> class Io; }
namespace Protocol { class Message; }
namespace Util { class Logger; }

namespace Backup {

class ServiceLoop {
private:
	/* self-from-this pattern.  */
	std::weak_ptr<ServiceLoop> weak_self;
	std::shared_ptr<ServiceLoop> get_self() const {
		return std::shared_ptr<ServiceLoop>(weak_self);
	}

	Util::Logger& logger;
	int fd_num;
	Daemon::Messenger messenger;
	Secp256k1::PubKey cid;

	bool timedout;

	ServiceLoop( Util::Logger& logger_
		   , Daemon::Breaker& breaker_
		   , int fd_num_
		   , Net::SocketFd fd_
		   , Noise::Encryptor enc_
		   , Secp256k1::PubKey cid_
		   ) : logger(logger_)
		     , fd_num(fd_num_)
		     , messenger( logger_
				, breaker_
				, std::move(fd_)
				, std::move(enc_)
				)
		     , cid(std::move(cid_))
		     , timedout(false)
		     { }

public:
	static
	std::shared_ptr<ServiceLoop> create( Util::Logger& logger
					   , Daemon::Breaker& breaker
					   , Net::SocketFd fd
					   , Noise::Encryptor enc
					   , Secp256k1::PubKey cid
					   ) {
		auto fd_num = fd.get();
		auto rv = std::shared_ptr<ServiceLoop>(
			new ServiceLoop( logger, breaker
				       , fd_num
				       , std::move(fd)
				       , std::move(enc)
				       , std::move(cid)
				       )
		);
		rv->weak_self = rv;
		return rv;
	}

	/* Enter the loop to continuously serve to the client.  */
	Ev::Io<int> enter_loop();
private:
	/* The actual main loop.  */
	Ev::Io<int> loop();
	/* Called when we want to ping the client to keep the
	 * connection alive.  */
	Ev::Io<int> ping();
	/* Called to dispatch on the message.  */
	Ev::Io<int> dispatch_msg(Protocol::Message);
	/* Used to get a text description of the message.  */
	std::string describe_msg(Protocol::Message const&);
	/* Called when the client pinged us to keep the connection
	 * alive.  */
	Ev::Io<int> pong();
};

}

#endif /* CLDCB_SERVER_BACKUP_SERVICELOOP_HPP */

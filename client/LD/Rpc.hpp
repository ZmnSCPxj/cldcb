#ifndef CLDCB_CLIENT_LD_RPC_HPP
#define CLDCB_CLIENT_LD_RPC_HPP

#include<memory>
#include<string>

namespace Util { class Logger; }
namespace Jsmn { class Object; }

namespace LD {

/* A synchronous interface to the LightningD RPC.

Note that since this is synchronous, it will block until the
RPC command completes, and multiple threads are likely to
massively confuse this object.
*/
class Rpc {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	~Rpc();
	Rpc() =delete;

	Rpc( Util::Logger& logger
	   , std::string const& rpc_filename = "lightning-rpc"
	   );

	Rpc(Rpc&&);
	Rpc(Rpc const&) =delete;
};

}

#endif /* CLDCB_CLIENT_LD_RPC_HPP */

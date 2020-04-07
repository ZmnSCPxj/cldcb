#ifndef CLDCB_CLIENT_PLUGIN_SINGLE_CREATE_HPP
#define CLDCB_CLIENT_PLUGIN_SINGLE_CREATE_HPP

#include<memory>

namespace Net { class Connector; }
namespace Plugin { class ServerIf; }
namespace Plugin { class ServerSpec; }
namespace Plugin { class Setup; }
namespace Util { class Logger; }

namespace Plugin { namespace Single {

/* Factory for single-server interfaces.  */
std::unique_ptr<Plugin::ServerIf>
create( Util::Logger& logger
      , Net::Connector& connector
      , Plugin::Setup const& setup
      , Plugin::ServerSpec const& server_spec
      );

}}

#endif /* CLDCB_CLIENT_PLUGIN_SINGLE_CREATE_HPP */

#ifndef CLDCB_CLIENT_PLUGIN_MAKE_NET_CONNECTOR_HPP
#define CLDCB_CLIENT_PLUGIN_MAKE_NET_CONNECTOR_HPP

#include<memory>

namespace Net { class Connector; }
namespace Plugin { class Setup; }

namespace Plugin {

/* Creates a connector based on the setup.  */
std::unique_ptr<Net::Connector>
make_net_connector(Plugin::Setup const&);

}

#endif /* CLDCB_CLIENT_PLUGIN_MAKE_NET_CONNECTOR_HPP */

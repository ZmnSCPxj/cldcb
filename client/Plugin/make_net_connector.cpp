#include"Plugin/make_net_connector.hpp"
#include"Net/Connector.hpp"
#include"Net/DirectConnector.hpp"
#include"Net/ProxyConnector.hpp"
#include"Plugin/Setup.hpp"
#include"Util/make_unique.hpp"

namespace Plugin {

std::unique_ptr<Net::Connector>
make_net_connector(Plugin::Setup const& setup) {
	auto ret = std::unique_ptr<Net::Connector>(nullptr);

	ret = Util::make_unique<Net::DirectConnector>();

	if (setup.has_proxy)
		ret = Util::make_unique<Net::ProxyConnector>
			( std::move(ret)
			, setup.proxy_host
			, setup.proxy_port
			);

	return ret;
}

}

#include<memory>
#include"Daemon/AcceptHandler.hpp"
#include"Daemon/Connection.hpp"
#include"Ev/Io.hpp"
#include"Net/SocketFd.hpp"

namespace Daemon {

Ev::Io<int> AcceptHandler::operator()(Net::SocketFd fd) {
	auto connection = std::make_shared<Daemon::Connection>( logger
							      , breaker
							      , identity
							      , prologue
							      , std::move(fd)
							      );
	return Daemon::Connection::new_connection(std::move(connection));
}

}

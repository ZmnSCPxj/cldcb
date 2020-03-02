#include"Main.hpp"
#include"Server/AddRemove.hpp"
#include"Server/Daemon.hpp"
#include"Server/Stop.hpp"

int main(int argc, char **argv) {
	return Main("cldcb-server")
	     . add_method("daemon", "Start server daemon.", Server::Daemon())
	     . add_method("add", "Add allowed clients."
			 , Server::AddRemove(Server::AddRemove::AddMode)
			 )
	     . add_method("remove", "Remove allowed clients."
			 , Server::AddRemove(Server::AddRemove::RemoveMode)
			 )
	     . add_method("stop", "Stop server daemon.", Server::Stop())
	     . main(argc, argv)
	     ;;
}

#include"Main.hpp"
#include"Server/Add.hpp"
#include"Server/Daemon.hpp"

int main(int argc, char **argv) {
	return Main("cldcb-server")
	     . add_method("daemon", "Start server daemon.", Server::Daemon())
	     . add_method("add", "Add allowed clients.", Server::Add())
	     . main(argc, argv)
	     ;;
}

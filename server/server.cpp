#include"Main.hpp"
#include"Server/Daemon.hpp"

int main(int argc, char **argv) {
	return Main("cldcb-server")
	     . add_method("daemon", Server::Daemon())
	     . main(argc, argv)
	     ;;
}

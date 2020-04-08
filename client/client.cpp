#include"Client/NewClient.hpp"
#include"Main.hpp"

int main(int argc, char** argv) {
	return Main("cldcb-client")
	     . add_method( "newclient"
			 , "Create a keypair in a client "
			   "private key file."
			 , Client::NewClient()
			 )
	     . main(argc, argv)
	     ;
}

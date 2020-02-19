#ifndef CLDCB_CLIENT_PLUGIN_MAIN_HPP
#define CLDCB_CLIENT_PLUGIN_MAIN_HPP

#include<istream>
#include<memory>
#include<ostream>

namespace Plugin {

class Main {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	Main() =delete;
	Main(Main&&);
	~Main();

	Main( std::istream& stdin, std::ostream& stdout
	    , int argc, char** argv
	    );

	int run();
};

}

#endif /* CLDCB_CLIENT_PLUGIN_MAIN_HPP */

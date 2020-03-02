#ifndef CLDCB_SERVER_SERVER_ADD_HPP
#define CLDCB_SERVER_SERVER_ADD_HPP

#include<string>
#include<vector>

namespace Server {

class Add {
public:
	int operator()(std::vector<std::string>);
};

}

#endif /* CLDCB_SERVER_SERVER_ADD_HPP */

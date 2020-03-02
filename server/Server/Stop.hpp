#ifndef CLDCB_SERVER_SERVER_STOP_HPP
#define CLDCB_SERVER_SERVER_STOP_HPP

#include<string>
#include<vector>

namespace Server {

class Stop {
public:
	int operator()(std::vector<std::string>);
};

}

#endif /* CLDCB_SERVER_SERVER_STOP_HPP */

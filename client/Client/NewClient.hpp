#ifndef CLDCB_CLIENT_CLIENT_NEWCLIENT_HPP
#define CLDCB_CLIENT_CLIENT_NEWCLIENT_HPP

#include<string>
#include<vector>

namespace Client {

class NewClient {
private:

public:
	int operator()(std::vector<std::string> params);
};

}

#endif /* CLDCB_CLIENT_CLIENT_NEWCLIENT_HPP */

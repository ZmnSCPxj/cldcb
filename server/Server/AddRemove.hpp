#ifndef CLDCB_SERVER_SERVER_ADDREMOVE_HPP
#define CLDCB_SERVER_SERVER_ADDREMOVE_HPP

#include<string>
#include<vector>

namespace Server {

class AddRemove {
public:
	enum AddRemoveMode
	{ AddMode
	, RemoveMode
	};

private:
	AddRemoveMode mode;

public:
	explicit AddRemove(AddRemoveMode mode_) : mode(mode_) { }

	int operator()(std::vector<std::string>);
};

}

#endif /* CLDCB_SERVER_SERVER_ADDREMOVE_HPP */

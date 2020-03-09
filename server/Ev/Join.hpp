#ifndef CLDCB_SERVER_EV_JOIN_HPP
#define CLDCB_SERVER_EV_JOIN_HPP

#include<functional>
#include<list>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/* A join synchronization point: max_count concurrent Ev::Io tasks
 * must invoke the join() function before all of them return.
 */
class Join {
private:
	unsigned int max_num;
	unsigned int num;
	std::list<std::function<void(int)>> passes;

public:
	explicit Join(unsigned int max_num_ = 2)
		: max_num(max_num_)
		, num(0)
		, passes()
		{ }
	Ev::Io<int> join();
};

}

#endif /* CLDCB_SERVER_EV_JOIN_HPP */

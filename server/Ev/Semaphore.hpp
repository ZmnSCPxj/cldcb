#ifndef CLDCB_SERVER_EV_SEMAPHORE_HPP
#define CLDCB_SERVER_EV_SEMAPHORE_HPP

#include<functional>
#include<queue>

namespace Ev { template<typename a> class Io; }

namespace Ev {

/* A semaphore cannot go negative.  */
class Semaphore {
private:
	unsigned int count;
	std::queue<std::function<void(unsigned int)>> passes;

public:
	explicit Semaphore(unsigned int count_ = 1)
		: count(count_)
		{ }
	Semaphore(Semaphore&&) =delete;
	Semaphore(Semaphore const&) =delete;

	Ev::Io<unsigned int> wait();
	Ev::Io<unsigned int> signal();
};

}

#endif /* CLDCB_SERVER_EV_SEMAPHORE_HPP */


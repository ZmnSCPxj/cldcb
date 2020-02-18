#include<assert.h>
#include<memory>
#include<thread>
#include"Sync/MVar.hpp"
#include"Util/make_unique.hpp"

class Destructible {
private:
	static int count;

public:
	Destructible() { ++count; }
	Destructible(Destructible&&) { ++count; }
	Destructible(Destructible const&) =delete;
	~Destructible() { --count; }

	static void check_count() { assert(count == 0); }
};
int Destructible::count = 0;

int main() {
	Sync::MVar<std::unique_ptr<int>> mvi;
	assert(!mvi.is_filled());

	/* Simple put and take.  */
	mvi.put(Util::make_unique<int>(42));
	assert(mvi.is_filled());
	auto tmp = mvi.take();
	assert(tmp);
	assert(*tmp == 42);

	/* Destruction.  */
	{
		auto tmp = Util::make_unique<Sync::MVar<Destructible>>();
		/* Fill the MVar.  */
		{
			tmp->put(Destructible());
			assert(tmp->is_filled());
		}
		/* Destroy the MVar. */
		tmp.reset();
		/* All destructibles should be destroyed at this point.  */
		Destructible::check_count();
	}

	/* Simple multithreaded.  */
	{
		std::thread one([&mvi]() {
			for (auto i = int(0); i < 6; ++i)
				mvi.put(Util::make_unique<int>(i));
		});
		std::thread two([&mvi]() {
			auto sum = int(0);
			for (auto i = int(0); i < 6; ++i) {
				auto dat = mvi.take();
				assert(dat);
				sum += *dat;
			}
			assert(sum == 0 + 1 + 2 + 3 + 4 + 5);
		});
		one.join();
		two.join();
	}

	return 0;
}

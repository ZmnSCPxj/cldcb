#include<assert.h>
#include<stdlib.h>
#include"Util/fork_test.hpp"

int main() {
	{
		/* Test failing child.  */
		auto res = Util::fork_test([]() {
			assert(true == false);
		});
		assert(res);
		assert(*res != 0);
	}
	{
		/* Test passing child from parent.  */
		auto res = Util::fork_test([]() {
			assert(true == true);
			assert(false == false);
			exit(0);
		});
		assert(!res);
	}
	{
		/* Test passing child from child.  */
		auto res = Util::fork_test([]() {
			assert(true == true);
			assert(false == false);
		});
		if (res) {
			assert(*res == 0);
		}
	}

}

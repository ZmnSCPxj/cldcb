#include<new>
#include<stdlib.h>

unsigned long count = 0;

void* operator new(unsigned long size) {
	++count;
	return malloc(size);
}
void operator delete(void* p) noexcept {
	--count;
	free(p);
}
/* For some reason, GCC 9, on -O2, uses this function to
 * delete an object it allocated with the above operator
 * new, which massively confuses our count (and raises
 * an error in valgrind).
 * It is not used on GCC 9 on -00 though....
 */
void operator delete(void* p, unsigned long) noexcept {
	--count;
	free(p);
}

#include<assert.h>
#include"Ev/Io.hpp"
#include"Ev/start.hpp"
#include"Ev/yield.hpp"

#define NUM_LOOPS 200

#define MAX_COUNT 10

Ev::Io<int> loop(unsigned long n) {
	return Ev::yield().then<int>([n](int) {
		/* Number of allocated objects should remain small.  */
		assert(count < MAX_COUNT);
		if (n == 0) {
			return Ev::lift_io(0);
		} else {
			return loop(n - 1);
		}
	});
}

int main() {
	return Ev::start(loop(NUM_LOOPS));
}


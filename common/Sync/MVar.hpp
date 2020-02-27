#ifndef CLDCB_COMMON_SYNC_MVAR_HPP
#define CLDCB_COMMON_SYNC_MVAR_HPP

#include<condition_variable>
#include<cstdint>
#include<mutex>
#include<utility>

namespace Sync {

template<typename a>
class MVar {
private:
	/* Must be held.  */
	std::mutex mtx;
	/* Wait on this if filled is the wrong value.  */
	std::condition_variable cv;

	/* TODO: This should really be std::optional.  */
	/* Set if storage is a valid object.  */
	bool filled;
	/* Storage space for an object.  */
	std::uint8_t storage[sizeof(a)];

	a* get_storage() {
		return reinterpret_cast<a*>(storage);
	}

	void destroy_storage() {
		get_storage()->~a();
		filled = false;
	}

public:
	MVar() {
		filled = false;
	}
	MVar (a value) {
		filled = true;
		new(get_storage()) a(std::move(value));
	}
	MVar(MVar&& o) {
		std::unique_lock<std::mutex> g(o.mtx);
		if (o.filled) {
			new(get_storage()) a(std::move(*o.get_storage()));
			filled = true;
		} else {
			filled = false;
		}
	}
	~MVar() {
		if (filled)
			destroy_storage();
	}

	a take() {
		std::unique_lock<std::mutex> g(mtx);
		while (!filled)
			cv.wait(g);
		auto ret = a(std::move(*get_storage()));
		destroy_storage();
		cv.notify_all();
		return ret;
	}
	void put(a value) {
		std::unique_lock<std::mutex> g(mtx);
		while (filled)
			cv.wait(g);
		new(get_storage()) a(std::move(value));
		filled = true;
		cv.notify_all();
	}

	/* NOTE! This is only exposed for unit testing.  Users of
	 * Sync::MVar must ***NEVER*** use this.
	 * FIXME: Guard with some kind of test-only define.
	 */
	bool is_filled() {
		std::unique_lock<std::mutex> g(mtx);
		return filled;
	}
};

}

#endif /* CLDCB_COMMON_SYNC_MVAR_HPP */

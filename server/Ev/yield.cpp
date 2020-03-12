#include<ev.h>
#include<utility>
#include"Ev/Io.hpp"
#include"Ev/yield.hpp"
#include"Util/make_unique.hpp"

namespace {

void yield_handler(EV_P_ ev_idle* raw_idler, int) {
	/* Re-acquire responsibility.  */
	auto idler = std::unique_ptr<ev_idle>(raw_idler);
	ev_idle_stop(EV_A_ idler.get());
	auto ppass = std::unique_ptr<std::function<void()>>();
	ppass.reset((std::function<void()>*)idler->data);

	/* Move off the pass function.  */
	auto pass = std::move(*ppass);

	/* Release resources.  */
	idler = nullptr;
	ppass = nullptr;

	pass();
}

class YieldWrapper {
private:
	std::function<void(int)> pass;
public:
	YieldWrapper(std::function<void(int)> pass_)
		: pass(std::move(pass_))
		{ }
	YieldWrapper(YieldWrapper&&) =default;
	YieldWrapper(YieldWrapper const&) =default;

	void operator()() const { pass(0); }
};

}

namespace Ev { 

namespace Detail {

void yield_pass(std::function<void()> pass) {
	/* Acquire resources.  */
	auto ppass = Util::make_unique<std::function<void()>>(
		std::move(pass)
	);
	auto idler = Util::make_unique<ev_idle>();
	ev_idle_init(idler.get(), &yield_handler);

	/* Release responsibility to C code.  */
	idler->data = ppass.release();
	ev_idle_start(EV_DEFAULT_ idler.release());
}

}

Io<int> yield() {
	return Io<int>([]( std::function<void(int)> pass
			 , std::function<void(std::exception)> fail
			 ) {
		Detail::yield_pass(YieldWrapper(std::move(pass)));
	});
}

}


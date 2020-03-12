#include"Ev/Io.hpp"
#include"Ev/Semaphore.hpp"
#include"Ev/yield.hpp"

namespace {

class SignalWrapper {
private:
	std::function<void(unsigned int)> pass;

public:
	SignalWrapper(std::function<void(unsigned int)> pass_)
		: pass(std::move(pass_))
		{ }
	SignalWrapper(SignalWrapper&&) =default;
	SignalWrapper& operator=(SignalWrapper&&) =default;
	SignalWrapper(SignalWrapper const&) =default;
	SignalWrapper& operator=(SignalWrapper const&) =default;

	void operator()() { pass(0); }
};

}

namespace Ev {

Ev::Io<unsigned int> Semaphore::wait() {
	return Ev::Io<unsigned int>([ this
				    ]( std::function<void(unsigned int)> pass
				     , std::function<void(std::exception)> fail
				     ) {
		if (count == 0) {
			passes.emplace(std::move(pass));
		} else {
			--count;
			pass(count);
		}
	});
}
Ev::Io<unsigned int> Semaphore::signal() {
	return Ev::Io<unsigned int>([ this
				    ]( std::function<void(unsigned int)> pass
				     , std::function<void(std::exception)> fail
				     ) {
		if (count == 0 && !passes.empty()) {
			auto pass2 = std::move(passes.front());
			passes.pop();
			Ev::Detail::yield_pass(SignalWrapper(std::move(pass2)));
		} else
			++count;
		pass(count);
	});
}

}

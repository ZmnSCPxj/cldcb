#include"Ev/Io.hpp"
#include"Ev/Join.hpp"
#include"Ev/yield.hpp"

namespace Ev {

Ev::Io<int> Join::join() {
	return Ev::Io<int>([this]( std::function<void(int)> pass
				 , std::function<void(std::exception)> fail
				 ) {
		++num;
		passes.push_back(std::move(pass));
		if (num == max_num) {
			auto my_passes = std::move(passes);
			num = 0;
			for (auto& p : my_passes) {
				try {
					p(0);
				} catch (...) { }
				p = nullptr;
			}
		}
	}).then<int>([](int) { return Ev::yield(); });
}

}

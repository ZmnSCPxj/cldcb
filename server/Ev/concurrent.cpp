#include<ev.h>
#include<iostream>
#include<memory>
#include"Ev/Io.hpp"
#include"Ev/concurrent.hpp"
#include"Util/make_unique.hpp"

namespace {

void concurrent_idle_handler(EV_P_ ev_idle *raw_idler, int revents) {
	/* Recover control of idler back from C.  */
	auto idler = std::unique_ptr<ev_idle>(raw_idler);
	ev_idle_stop(EV_A_ idler.get());

	/* Recover control of io back from C.  */
	auto raw_io_ptr = (Ev::Io<int>*) idler->data;
	auto io_ptr = std::unique_ptr<Ev::Io<int>>(raw_io_ptr);

	io_ptr->run([](int) { /* Do nothing*/ }, [](std::exception e) {
		std::cerr << "Unhandled exception in concurrent task!" << std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << "...main loop continuing..." << std::endl;
	});
}

}

namespace Ev {

Io<int> concurrent(Io<int> io) {
	return Io<int>([io]( std::function<void(int)> pass
			   , std::function<void(std::exception)> fail
			   ) {
		bool passed = false;
		try {
			auto io_ptr = Util::make_unique<Io<int>>(std::move(io));

			auto idler = Util::make_unique<ev_idle>();
			ev_idle_init(idler.get(), &concurrent_idle_handler);
			/* Hand over control of io to C.  */
			idler->data = (void*) io_ptr.release();
			/* Hand over control of idler to C.  */
			ev_idle_start(EV_DEFAULT_ idler.release());

			passed = true;
		} catch (std::exception const& e) {
			fail(e);
		}
		if (passed)
			pass(0);
	});
}

}

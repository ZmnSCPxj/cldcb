#ifndef CLDCB_SERVER_EV_IO_HPP
#define CLDCB_SERVER_EV_IO_HPP

#include<exception>
#include<functional>
#include<memory>

/* Ev::Io is a monad, which is just a monoid over the category of endofunctors. */

namespace Ev {

template<typename a>
class Io {
public:
	typedef
	std::function<void ( std::function<void (a)>
			   , std::function<void (std::exception)>
			   )> CoreFunc;

private:
	CoreFunc core;

public:
	/* Copyable.  */
	explicit Io(CoreFunc core_) : core(std::move(core_)) { }

	/* Unfortunately C++ type derivation is not clever enough
	 * to match Haskell.
	 * So you need to use this by .then<b>([](a) { ... return Io<b>(...); })
	 */
	/* IO a -> (a -> IO b) -> IO b */
	template<typename b>
	Io<b> then(std::function<Io<b>(a)> f) {
		/* Continuation Monad.  */
		auto core_copy = core;
		return Io<b>([ core_copy
			     , f
			     ]( std::function<void(b)> pass
			      , std::function<void(std::exception)> fail
			      ) {
			try {
				core_copy( [f, pass, fail](a value) {
					f(std::move(value)).core(pass, fail);
				}
					 , fail
					 );
			} catch (std::exception const& e) {
				fail(e);
			}
		});
	}

	/* Catches failures and replaces the return value.  */
	Io<a> catching(std::function<Io<a>(std::exception)> handler) {
		auto core_copy = core;
		return Io<a>([ core_copy
			     , handler
			     ]( std::function<void(a)> pass
			      , std::function<void(std::exception)> fail
			      ) {
			try {
				core_copy( pass
					 , [handler, pass, fail](std::exception e) {
					handler(std::move(e)).core(pass, fail);
				}
					 );
			} catch (std::exception const& e) {
				handler(e).run(pass, fail);
			}
		});
	}

	/* Execute the code with the given callbacks.
	 * Never throws, but only because it instead calls
	 * the given fail callback.
	 */
	void run( std::function<void(a)> pass
		, std::function<void(std::exception)> fail
		) const noexcept {
		auto completed = std::make_shared<bool>(false);
		auto sub_pass = [completed, pass](a value) {
			if (!*completed) {
				*completed = true;
				pass(std::move(value));
			}
		};
		auto sub_fail = [completed, fail](std::exception e) {
			if (!*completed) {
				*completed = true;
				fail(std::move(e));
			}
		};
		try {
			core(std::move(sub_pass), std::move(sub_fail));
		} catch (std::exception const& e) {
			sub_fail(e);
		}
	}

};

/* a must be at least moveable.  */
/* a -> IO a */
template<typename a>
Io<a> lift_io(a value) {
	auto container = std::make_shared<a>(std::move(value));
	return Io<a>([container]( std::function<void(a)> pass
				, std::function<void(std::exception)> fail
				) {
		pass(std::move(*container));
	});
}

}

#endif /* CLDCB_SERVER_EV_IO_HPP */

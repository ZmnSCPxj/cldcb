#include<thread>
#include"LD/Writer.hpp"
#include"Sync/MVar.hpp"
#include"Util/make_unique.hpp"

namespace LD {

class Writer::Impl {
private:
	std::thread background;
	Sync::MVar<std::unique_ptr<std::string>> channel;

public:
	Impl(std::ostream& os)
		: background([this, &os]() {
			for (;;) {
				auto ps = channel.take();
				if (!ps) {
					os.flush();
					break;
				}
				os << *ps << std::endl;
				/* We will block at the next loop iteration,
				 * so flush the output stream now.
				 */
				os.flush();
			}
		  })
		, channel()
		{ }
	~Impl() {
		channel.put(nullptr);
		background.join();
	}
	Impl(Impl&&) =delete;
	Impl(Impl const&) =delete;

	void write(std::string const& s) {
		channel.put(Util::make_unique<std::string>(s));
	}
};

Writer::~Writer() { }
Writer::Writer(std::ostream& os)
	: pimpl(Util::make_unique<Impl>(os)) { }
Writer::Writer(Writer&& o)
	: pimpl(std::move(o.pimpl)) { }

void Writer::write(std::string const& s) {
	if (!pimpl)
		return;
	pimpl->write(s);
}

}

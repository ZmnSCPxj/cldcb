#ifndef CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP
#define CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP

#include<cstdint>
#include<memory>
#include<string>
#include<vector>

namespace LD { class DbWrite; }
namespace Plugin { class DbFileReader; }
namespace Plugin { class ServerIf; }
namespace Plugin { class Setup; }

namespace Plugin {

class DbWriteHandler {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	DbWriteHandler() =delete;
	DbWriteHandler(DbWriteHandler&&);
	~DbWriteHandler();
	DbWriteHandler& operator=(DbWriteHandler&& o) {
		o.pimpl.swap(pimpl);
		return *this;
	}
	explicit DbWriteHandler(Setup&, ServerIf&, DbFileReader&);

	/* Return true if handled OK, false if backing up failed.  */
	bool handle(LD::DbWrite const&);
};

}

#endif /* CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP */

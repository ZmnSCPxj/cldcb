#ifndef CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP
#define CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP

#include<cstdint>
#include<string>
#include<vector>

namespace LD { class DbWrite; }

namespace Plugin {

class DbWriteHandler {
public:
	virtual ~DbWriteHandler() { }

	/* Return true if handled OK, false if backing up failed.  */
	virtual bool handle( LD::DbWrite const&
			   ) =0;
};

}

#endif /* CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP */

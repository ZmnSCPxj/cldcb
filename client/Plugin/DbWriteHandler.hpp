#ifndef CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP
#define CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP

#include<cstdint>
#include<string>
#include<vector>

namespace Plugin {

class DbWriteHandler {
public:
	virtual ~DbWriteHandler() { }

	/* Return true if handled OK, false if backing up failed.  */
	virtual bool handle( std::uint32_t data_version
			   , std::vector<std::string> writes
			   ) =0;
};

}

#endif /* CLDCB_COMMON_PLUGIN_DBWRITEHANDLER_HPP */

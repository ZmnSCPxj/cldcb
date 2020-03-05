#ifndef CLDCB_SERVER_BACKUP_DATAREADER_HPP
#define CLDCB_SERVER_BACKUP_DATAREADER_HPP

#include<cstdint>
#include<memory>
#include<utility>
#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Backup {

class DataReader {
public:
	virtual ~DataReader() { }

	/* Returns nullptr if an error occurred.
	 * Returns a vector of size 0 if ended reupload data.
	 */
	virtual
	Ev::Io<std::unique_ptr<std::vector<std::uint8_t>>>
	backedup_reupload_chunk() =0;

	/* Returns nullptr if an error occurred.
	 * The paired uint32_t is the data_version.
	 * Returns a vector of size 0 if ended *all* chunks
	 * with all `data_version`s.
	 */
	virtual
	Ev::Io<
		std::unique_ptr<std::pair< std::uint32_t
					 , std::vector<std::uint8_t>
					 >>
	      >
	backedup_incremental_chunk() =0;
};

}

#endif /* CLDCB_SERVER_BACKUP_DATAREADER_HPP */

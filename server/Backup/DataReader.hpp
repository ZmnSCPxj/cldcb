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

	enum IncrementType
	{ New
	, Chunk
	, EndAll
	};

	struct IncrementMsg {
		IncrementType type;
		/* Filled in if type == New.  */
		std::uint32_t data_version;
		/* Filled in if type == Chunk.  */
		std::vector<std::uint8_t> chunk;

		IncrementMsg() =default;
		IncrementMsg(IncrementMsg&&) =default;
		IncrementMsg& operator=(IncrementMsg&&) =default;
		IncrementMsg(IncrementMsg const&) =default;
		IncrementMsg& operator=(IncrementMsg const&) =default;
		IncrementMsg( IncrementType type_
			    , std::uint32_t data_version_
			    , std::vector<std::uint8_t> chunk_
			    ) : type(type_)
			      , data_version(data_version_)
			      , chunk(std::move(chunk_))
			      {  }
	};

	/* Returns nullptr if an error occurred.
	 * Else returns the result.
	 */
	virtual
	Ev::Io<std::unique_ptr<IncrementMsg>>
	backedup_incremental_get() =0;
};

}

#endif /* CLDCB_SERVER_BACKUP_DATAREADER_HPP */

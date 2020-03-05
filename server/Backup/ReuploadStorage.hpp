#ifndef CLDCB_SERVER_BACKUP_REUPLOADSTORAGE_HPP
#define CLDCB_SERVER_BACKUP_REUPLOADSTORAGE_HPP

#include<cstdint>
#include<vector>

namespace Ev { template<typename a> class Io; }

namespace Backup {

class ReuploadStorage {
public:
	virtual ~ReuploadStorage() { }

	/* Store a chunk of reupload data.
	 * Return false in case there is a problem writing
	 * the chunk.
	 * Chunks must NOT be size 0.
	 */
	virtual
	Ev::Io<bool> reupload_chunk(std::vector<std::uint8_t> chunk) =0;

	/* Finish the reupload chunks.  */
	virtual
	Ev::Io<bool> reupload_end() =0;
};

}

#endif /* CLDCB_SERVER_BACKUP_REUPLOADSTORAGE_HPP */

#ifndef CLDCB_SERVER_BACKUP_INCREMENTALSTORAGE_HPP
#define CLDCB_SERVER_BACKUP_INCREMENTALSTORAGE_HPP

#include<cstdint>
#include<memory>
#include<vector>

namespace Backup { class ReuploadStorage; }
namespace Ev { template<typename a> class Io; }

namespace Backup {

class IncrementalStorage {
public:
	virtual ~IncrementalStorage() { }

	/* Flag to determine if we will eventually ask for a reupload
	 * later.
	 * This flag should not change per instance of this object;
	 * if it returns a particular value, it should always
	 * return that value.
	 */
	virtual
	bool will_response_reupload() const =0;

	/* Send incremental chunk to the storage.
	 * Return false if some problem occurred during write.
	 * Chunks must NOT be size 0.
	 */
	virtual
	Ev::Io<bool> incremental_chunk(std::vector<std::uint8_t> chunk) =0;
	/* Finish the incremental chunks.
	 * Return false if some problem occurred during write.
	 */
	virtual
	Ev::Io<bool> incremental_end() =0;

	/* Get the reupload storage interface, if will_response_reupload()
	 * returns true.
	 * Not called if will_response_reupload() returns false.
	 * Must be called after incremental_end() is called.
	 */
	virtual
	Ev::Io<std::unique_ptr<Backup::ReuploadStorage>>
	get_response_storage() =0;
};

}

#endif /* CLDCB_SERVER_BACKUP_INCREMENTALSTORAGE_HPP */

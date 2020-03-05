#ifndef CLDCB_SERVER_BACKUP_DATASTORAGE_HPP
#define CLDCB_SERVER_BACKUP_DATASTORAGE_HPP

#include<cstdint>
#include<memory>
#include<unordered_map>
#include"Secp256k1/PubKey.hpp"

namespace Backup { class DataReader; }
namespace Backup { class IncrementalStorage; }
namespace Ev { template<typename a> class Io; }

namespace Backup {

/* Abstract class representing the storage for backed up data.  */
class DataStorage {
public:
	virtual ~DataStorage() { }

	/* Requests to send an incremental update.
	 * If it returns nullptr then it means the given client
	 * is not authorized to send updates.
	 */
	virtual
	Ev::Io<std::unique_ptr<Backup::IncrementalStorage>>
	request_incremental( Secp256k1::PubKey const& cid
			   , std::uint32_t data_version
			   ) =0;

	/* Requests to get the backup data from a specific
	 * client id.
	 * If the given pubkey is currently registered as
	 * connected, return nullptr (i.e. we cannot read
	 * from an archive while it is actively being
	 * written to by the client).
	 * If the given cid does not have data associated
	 * with it, return nullptr as well.
	 */
	virtual
	Ev::Io<std::unique_ptr<Backup::DataReader>>
	request_backup_data(Secp256k1::PubKey const& cid) =0;

	/* Call to register/unregister that a particular
	 * client is currently connected to the server.
	 * We should not return backup data while the
	 * client is connected (that implies the client is
	 * still alive, so why should it be asking for
	 * data it is still supposedly writing to...?).
	 * You should RAII registration/unregistration.
	 */
	void connect_cid(Secp256k1::PubKey const&);
	void disconnect_cid(Secp256k1::PubKey const&);
protected:
	/* Derivatives of this base class can use this
	 * to check if a CID is currently connected.
	 */
	bool is_connected_cid(Secp256k1::PubKey const&);
private:
	std::unordered_map<Secp256k1::PubKey, std::size_t> connected_cids;
};

}

#endif /* CLDCB_SERVER_BACKUP_DATASTORAGE_HPP */

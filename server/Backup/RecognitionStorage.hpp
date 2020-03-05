#ifndef CLDCB_SERVER_BACKUP_RECOGNITIONSTORAGE_HPP
#define CLDCB_SERVER_BACKUP_RECOGNITIONSTORAGE_HPP

#include<cstdint>
#include<vector>

namespace Ev { template<typename a> class Io; }
namespace Secp256k1 { class PubKey; }

namespace Backup {

/* Abstract class representing the storage for recognition codes.  */
class RecognitionStorage {
public:
	virtual ~RecognitionStorage() { }

	/* Given a public key and a 64-byte recognition code.
	 * Return true if the given pubkey is authorized and
	 * we have saved the recognition code, false if the
	 * pubkey is unauthorized.
	 */
	virtual
	Ev::Io<bool> give_recognition_code( Secp256k1::PubKey const&
					  , std::vector<std::uint8_t> const&
					  ) =0;

	/* Returns a vector of all currently known
	 * recognition codes.
	 * Each recognition code is 97 bytes.
	 */
	virtual
	Ev::Io<std::vector<std::vector<std::uint8_t>>>
	request_recognition_codes() =0;
};

}

#endif /* CLDCB_SERVER_BACKUP_RECOGNITIONSTORAGE_HPP */

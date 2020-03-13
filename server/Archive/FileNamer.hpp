#ifndef CLDCB_SERVER_ARCHIVE_FILENAMER_HPP
#define CLDCB_SERVER_ARCHIVE_FILENAMER_HPP

#include<string>

namespace Secp256k1 { class PubKey; }

namespace Archive {

/* Abstract class to generate the filename from the given
 * pubkey.
 */
class FileNamer {
public:
	virtual ~FileNamer() { }

	virtual
	std::string
	get_archive_filename(Secp256k1::PubKey const&) const =0;
	virtual
	std::string
	get_temp_reupload_filename(Secp256k1::PubKey const&) const =0;
	virtual
	std::string
	get_temp_incremental_filename(Secp256k1::PubKey const&) const =0;

	/* Default implementation.  */
	class Default;
};

class FileNamer::Default : public FileNamer {
public:
	std::string
	get_archive_filename(Secp256k1::PubKey const&) const override;
	std::string
	get_temp_reupload_filename(Secp256k1::PubKey const&) const override;
	std::string
	get_temp_incremental_filename(Secp256k1::PubKey const&) const override;
};

}

#endif /* CLDCB_SERVER_ARCHIVE_FILENAMER_HPP */

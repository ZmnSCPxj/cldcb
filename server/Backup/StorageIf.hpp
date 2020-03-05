#ifndef CLDCB_SERVER_BACKUP_STORAGEIF_HPP
#define CLDCB_SERVER_BACKUP_STORAGEIF_HPP

#include"Backup/DataStorage.hpp"
#include"Backup/RecognitionStorage.hpp"

namespace Backup {

class StorageIf : public Backup::DataStorage
		, public Backup::RecognitionStorage
		{ };

}

#endif /* CLDCB_SERVER_BACKUP_STORAGEIF_HPP */

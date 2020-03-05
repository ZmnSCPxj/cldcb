#include<assert.h>
#include<utility>
#include"Backup/DataStorage.hpp"

namespace Backup {

void
DataStorage::connect_cid(Secp256k1::PubKey const& cid) {
	auto it = connected_cids.find(cid);
	if (it == connected_cids.end())
		connected_cids.insert(std::make_pair(cid, 1));
	else
		++it->second;
}
void
DataStorage::disconnect_cid(Secp256k1::PubKey const& cid) {
	auto it = connected_cids.find(cid);
	assert(it != connected_cids.end());
	--it->second;
	if (it->second == 0)
		connected_cids.erase(it);
}

bool
DataStorage::is_connected_cid(Secp256k1::PubKey const& cid) {
	auto it = connected_cids.find(cid);
	if (it == connected_cids.end())
		return false;
	assert(it->second != 0);
	return true;
}

}

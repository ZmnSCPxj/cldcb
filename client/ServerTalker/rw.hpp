#ifndef CLDCB_CLIENT_SERVERTALKER_RW_HPP
#define CLDCB_CLIENT_SERVERTALKER_RW_HPP

#include<cstdint>
#include<utility>

namespace ServerTalker {

/* Return true if successful, false if not.  */
bool write_all(int fd, void const* p, std::size_t size);

/* Return true if successful, false if not.
 * size is modified depending on actual number of
 * bytes read.
 * It is possible to fail with some number of
 * bytes already read.
 */
bool read_all(int fd, void* p, std::size_t& size);

}

#endif /* CLDCB_CLIENT_SERVERTALKER_RW_HPP */

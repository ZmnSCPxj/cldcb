#ifndef CLDCB_SERVER_SERVER_CHANGE_CLIENTS_HPP
#define CLDCB_SERVER_SERVER_CHANGE_CLIENTS_HPP

#include<functional>
#include<string>
#include<unordered_set>

namespace Secp256k1 { class PubKey; }
namespace Util { class Logger; }

namespace Server {

typedef std::unordered_set<Secp256k1::PubKey> ClientSet;

/* Locks the clients file for exclusive access.
 * Read the clients file, load it into a set,
 * call the given function (which is expected to
 * mutate the set), then atomically releases the
 * lock and writes the modified set.
 * Returns an empty string on success, an error
 * message on failure.
 * The current directory is presumed to be the
 * one that is being modified.
 * Not safe for multithreaded use; expects the
 * calling process to be a single process.
 */
std::string
change_clients( Util::Logger& logger
	      , std::string const& pidfile
	      , std::function<void(ClientSet&)> changer
	      );

}

#endif /* CLDCB_SERVER_SERVER_CHANGE_CLIENTS_HPP */

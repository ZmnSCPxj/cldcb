#include<errno.h>
#include<fcntl.h>
#include<sstream>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include"Client/NewClient.hpp"
#include"Net/Fd.hpp"
#include"Secp256k1/KeyPair.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Rw.hpp"
#include"Util/TermLogger.hpp"

namespace {

std::string clientify_pubkey(Secp256k1::PubKey const& pk) {
	auto os = std::ostringstream();
	os << pk;
	auto str = os.str();
	str[0] = 'c';
	return str;
}

}

namespace Client {

int NewClient::operator()(std::vector<std::string> params) {
	auto logger = Util::TermLogger();

	/* TODO: common options handling. */

	if (params.size() != 1) {
		logger.BROKEN("newclient must have one argument.");
		return 1;
	}

	auto& filename = params[0];

	logger.debug("Writing to: %s", filename.c_str());

	Secp256k1::Random rand;
	auto newclient = Secp256k1::KeyPair(rand);

	auto os = std::ostringstream();

	os << "# cid = " << clientify_pubkey(newclient.pub()) << std::endl;
	os << "# cpk = " << newclient.priv() << std::endl;

	auto fd = Net::Fd();
	do {
		fd = Net::Fd(open( filename.c_str()
				 , O_WRONLY
				 | O_TRUNC
				 | O_CREAT
				 , 0400
				 ));
	} while (!fd && errno == EINTR);
	if (!fd) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to open file %1$s: %2$s"
			     , filename.c_str()
			     , strerror(my_errno)
			     );
		return 1;
	}

	auto out = os.str();

	auto write_res = Util::Rw::write_all( fd.get()
					    , out.c_str()
					    , out.size()
					    );
	if (!write_res) {
		auto my_errno = errno;
		logger.BROKEN( "Failed to write to file %1$s: %2$s"
			     , filename.c_str()
			     , strerror(my_errno)
			     );
		return 1;
	}

	return 0;
}

}

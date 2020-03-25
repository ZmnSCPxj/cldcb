#include<sstream>
#include"Client/recognize_code.hpp"
#include"Crypto/Secret.hpp"
#include"LD/checkmessage.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/ecdh.hpp"

namespace Client {

std::string recognition_code_message( Secp256k1::PrivKey const& ecdh_sk
				    , Secp256k1::PubKey const& ecdh_pk
				    ) {
	auto ecdh = Secp256k1::ecdh(ecdh_sk, ecdh_pk);
	auto os = std::ostringstream();

	os << "DO NOT SIGN THIS UNLESS YOU WANT YOUR FUNDS TO BE STOLEN "
	   << ecdh
	    ;
	return os.str();
}

bool recognize_code( Secp256k1::PubKey const& signer
		   , Secp256k1::Signature const& recognition_code
		   , Secp256k1::PrivKey const& ecdh_sk
		   , Secp256k1::PubKey const& ecdh_pk
		   ) {
	auto message = recognition_code_message(ecdh_sk, ecdh_pk);
	return LD::checkmessage(signer, recognition_code, message);
}

}

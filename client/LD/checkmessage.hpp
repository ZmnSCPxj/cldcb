#ifndef CLDCB_CLIENT_LD_CHECKMESSAGE_HPP
#define CLDCB_CLIENT_LD_CHECKMESSAGE_HPP

#include<string>

namespace Secp256k1 { class PubKey; }
namespace Secp256k1 { class Signature; }

namespace LD {

bool checkmessage( Secp256k1::PubKey const&
		 , Secp256k1::Signature const&
		 , std::string const& message
		 );

}

#endif /* CLDCB_CLIENT_LD_CHECKMESSAGE_HPP */

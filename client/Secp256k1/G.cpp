#include"Secp256k1/Context.hpp"
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"

/* FIXME: Load G as a constant directly.  */
/* G = 0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798 */
namespace {

std::uint8_t one_a[32] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1
};

auto context = Secp256k1::Context();

auto one = Secp256k1::PrivKey::from_buffer(context, one_a);

}

namespace Secp256k1 {

PubKey G = PubKey(one);

}

#undef NDEBUG
#include<assert.h>
#include<functional>
#include"Crypto/Secret.hpp"
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Secp256k1/ecdh.hpp"

using Secp256k1::G;

int main() {
	assert(G == Secp256k1::PubKey("0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"));

	Secp256k1::Random random;

	auto a = Secp256k1::PrivKey(random);
	auto b = Secp256k1::PrivKey(random);

	/* Except with very low probability.  */
	assert(a != b);

	auto A = a * G;
	auto B = b * G;

	/* We already know a != b, so... */
	assert(A != B);
	/* Except with low probability.  */
	assert( std::hash<Secp256k1::PubKey>()(A)
	     != std::hash<Secp256k1::PubKey>()(B)
	      );

	assert(A == Secp256k1::PubKey(a));
	assert(B == Secp256k1::PubKey(b));
	assert(a * B == b * A);
	assert(A + B == (a + b) * G);
	assert((b * a) * G == a * B);

	/* From BOLT 8.  */
	auto P = Secp256k1::PubKey("028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7");
	auto s = Secp256k1::PrivKey("1212121212121212121212121212121212121212121212121212121212121212");
	auto ss = Secp256k1::ecdh(s, P);
	assert(ss == Crypto::Secret("1e2fb3c8fe8fb9f262f649f64d26ecf0f2c0a805a767cf02dc2d77a6ef1fdcc3"));

	/* Simple math.  */
	auto x = Secp256k1::PrivKey("0000000000000000000000000000000000000000000000000000000000000001");
	auto y = Secp256k1::PrivKey("0000000000000000000000000000000000000000000000000000000000000002");
	auto z = Secp256k1::PrivKey("0000000000000000000000000000000000000000000000000000000000000003");
	auto X = x * A;
	auto Y = y * A;
	auto Z = z * A;
	assert(X + Y == Z); /* 1 + 2 == 3 */
	assert(X + Y + Z == y * Z); /* 1 + 2 + 3 == 2 * 3 */
	assert(Z - X == Y); /* 3 - 1 == 2 */
	assert(x * Z == Z); /* 1 * 3 == 3 */

	/* Default-constructed privkey is just "1".  */
	assert((Secp256k1::PrivKey() * A) == A);

	return 0;
}

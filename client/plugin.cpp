#include<iostream>
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
#include"Secp256k1/ecdh.hpp"
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"

int main(int argc, char **argv) {
	Secp256k1::Random random;

	Secp256k1::PrivKey a(random);
	Secp256k1::PubKey A(a);

	Secp256k1::PrivKey b(random);
	Secp256k1::PubKey B(b);

	std::cout << Sha256::fun("a", 1) << std::endl;

	std::cout << Secp256k1::G << std::endl;

	std::cout << a << std::endl;
	std::cout << A << std::endl;
	std::cout << b << std::endl;
	std::cout << B << std::endl;

	std::cout << a * B << std::endl;
	std::cout << b * A << std::endl;

	std::cout << (a * B == b * A) << std::endl;
	std::cout << (Secp256k1::PubKey(a * b) == a * B) << std::endl;
	std::cout << (A + B == Secp256k1::PubKey(a + b)) << std::endl;

	std::cout << (A != B) << std::endl; /* With high probability anyway. */
	std::cout << (A + B != A) << std::endl;
	std::cout << (A + B != B) << std::endl;

	std::cout << (A == a * Secp256k1::G) << std::endl;

	/* From BOLT 8.  */
	Secp256k1::PubKey  P("028d7500dd4c12685d1f568b4c2b5048e8534b873319f3a8daa612b469132ec7f7");
	Secp256k1::PrivKey s("1212121212121212121212121212121212121212121212121212121212121212");
	auto ss = Secp256k1::ecdh(s, P);
	std::cout << ss << std::endl;
	std::cout << (ss == Secp256k1::PrivKey("1e2fb3c8fe8fb9f262f649f64d26ecf0f2c0a805a767cf02dc2d77a6ef1fdcc3")) << std::endl;

	std::cout << "Hello World." << std::endl;
	return 0;
}

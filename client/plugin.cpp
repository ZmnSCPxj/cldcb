#include<iostream>
#include"Secp256k1/G.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Random.hpp"
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

	std::cout << "Hello World." << std::endl;
	return 0;
}
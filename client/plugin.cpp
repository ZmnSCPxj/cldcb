#include<iostream>
#include"Secp256k1/Context.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"

int main(int argc, char **argv) {
	Secp256k1::Context secp;
	Secp256k1::Random random;
	Secp256k1::PrivKey priv(secp, random);

	std::cout << "Hello World." << std::endl;
	return 0;
}

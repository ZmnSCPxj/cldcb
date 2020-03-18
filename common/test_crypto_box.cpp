#include<assert.h>
#include"Crypto/Box/Sealer.hpp"
#include"Crypto/Box/Unsealer.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/PrivKey.hpp"
#include"Secp256k1/Random.hpp"

int main() {
	Secp256k1::Random rand;

	auto s = Secp256k1::PrivKey(rand);
	auto S = Secp256k1::PubKey(s);
	auto r = Secp256k1::PrivKey(rand);
	auto R = Secp256k1::PubKey(r);

	/* Simple seal/unseal.  */
	{
		auto sealer = Crypto::Box::Sealer(s, R);
		auto unsealer = Crypto::Box::Unsealer(S, r);
		auto plaintext = std::vector<std::uint8_t>();
		plaintext.push_back(0x01);
		plaintext.push_back(0x42);

		auto ciphertext = sealer.seal(plaintext);
		auto new_plaintext = unsealer.unseal(ciphertext);
		assert(new_plaintext);
		assert(*new_plaintext == plaintext);
	}

	/* Seal/unseal sequence.  */
	{
		auto number = 50;
		auto size = 1000;
		auto sealer = Crypto::Box::Sealer(s, R);
		auto unsealer = Crypto::Box::Unsealer(S, r);

		auto plaintexts = std::vector<std::vector<std::uint8_t>>();
		auto ciphertexts = std::vector<std::vector<std::uint8_t>>();

		for (auto n = 0; n < number; ++n) {
			auto plaintext = std::vector<std::uint8_t>(size);
			for (auto i = 0; i < size; ++i)
				plaintext[i] = rand.get();
			plaintexts.push_back(plaintext);
			ciphertexts.push_back(sealer.seal(plaintext));
		}

		for (auto n = 0; n < number; ++n) {
			auto new_plaintext = unsealer.unseal(ciphertexts[n]);
			assert(new_plaintext);
			assert(*new_plaintext == plaintexts[n]);
		}
	}

	/* Wrong privkey will not work.  */
	{
		auto size = 100;
		auto sealer = Crypto::Box::Sealer(s, R);
		auto q = Secp256k1::PrivKey(rand);
		auto unsealer = Crypto::Box::Unsealer(S, q);

		auto plaintext = std::vector<std::uint8_t>(size);
		for (auto& p : plaintext)
			p = rand.get();

		auto ciphertext = sealer.seal(plaintext);
		assert(!unsealer.unseal(ciphertext));
	}

	/* Same message to different sealers yield different ciphertexts.  */
	{
		auto size = 100;
		auto sealer1 = Crypto::Box::Sealer(s, R);
		auto sealer2 = Crypto::Box::Sealer(s, R);

		auto plaintext = std::vector<std::uint8_t>(size);
		for (auto& p : plaintext)
			p = rand.get();

		auto ciphertext1 = sealer1.seal(plaintext);
		auto ciphertext2 = sealer2.seal(plaintext);
		assert(ciphertext1 != ciphertext2);

		/* But will still yield the same plaintext.  */
		auto unsealer1 = Crypto::Box::Unsealer(S, r);
		auto unsealer2 = Crypto::Box::Unsealer(S, r);
		auto new_plaintext1 = unsealer1.unseal(ciphertext1);
		auto new_plaintext2 = unsealer2.unseal(ciphertext2);
		assert(new_plaintext1);
		assert(*new_plaintext1 == plaintext);
		assert(new_plaintext2);
		assert(*new_plaintext2 == plaintext);
	}

	/* Same message to the same sealer multiple times yields
	 * different ciphertexts.
	 */
	{
		auto size = 100;
		auto sealer = Crypto::Box::Sealer(s, R);

		auto plaintext = std::vector<std::uint8_t>(size);
		for (auto& p : plaintext)
			p = rand.get();

		auto ciphertext1 = sealer.seal(plaintext);
		auto ciphertext2 = sealer.seal(plaintext);
		auto ciphertext3 = sealer.seal(plaintext);
		assert(ciphertext1 != ciphertext2);
		assert(ciphertext2 != ciphertext3);
		assert(ciphertext3 != ciphertext1);
	}

	return 0;
}

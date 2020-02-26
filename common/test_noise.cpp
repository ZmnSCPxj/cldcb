#include<assert.h>
#include<cstdint>
#include<queue>
#include<vector>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/CipherState.hpp"
#include"Noise/Detail/hkdf2.hpp"
#include"Noise/Encryptor.hpp"
#include"Stream/SinkSource.hpp"
#include"Util/Str.hpp"

int main() {
	/* HKDF test vectors.  */
	/* From BOLT 8.  */
	{
		auto res = Noise::Detail::hkdf2( Crypto::Secret("cc2c6e467efc8067720c2d09c139d1f77731893aad1defa14f9bf3c48d3f1d31")
					       , Crypto::Secret("3fbdc101abd1132ca3a0ae34a669d8d9ba69a587e0bb4ddd59524541cf4813d8")
					       );
		assert(res.first == Crypto::Secret("728366ed68565dc17cf6dd97330a859a6a56e87e2beef3bd828a4c4a54d8df06"));
		assert(res.second == Crypto::Secret("9e0477f9850dca41e42db0e4d154e3a098e5a000d995e421849fcd5df27882bd"));
	}

	/* From BOLT 8.  */
	{
		auto res = Noise::Detail::hkdf2_zero( Crypto::Secret("919219dbb2920afa8db80f9a51787a840bcf111ed8d588caf9ab4be716e42b01")
						    );
		assert(res.first == Crypto::Secret("969ab31b4d288cedf6218839b27a3e2140827047f2c0f01bf5c04435d43511a9"));
		assert(res.second == Crypto::Secret("bb9020b8965f4df047e07f955f3c4b88418984aadc5cdb35096b9ea8fa5c3442"));
	}

	/* Encryption standard.  */
	/* From BOLT 8.  */
	{
		auto ck = Crypto::Secret("919219dbb2920afa8db80f9a51787a840bcf111ed8d588caf9ab4be716e42b01");
		auto sk = Crypto::Secret("969ab31b4d288cedf6218839b27a3e2140827047f2c0f01bf5c04435d43511a9");
		auto rk = Crypto::Secret("bb9020b8965f4df047e07f955f3c4b88418984aadc5cdb35096b9ea8fa5c3442");

		auto s = Noise::Detail::CipherState();
		s.initialize_key(sk);
		auto r = Noise::Detail::CipherState();
		r.initialize_key(rk);

		auto enc = Noise::Encryptor( Noise::Detail::CipherState(r)
					   , Noise::Detail::CipherState(s)
					   , ck
					   );
		/* The decryptor has the r and s swapped.
		 * The r is normally first because it is earlier in the alphabet.
		 */
		auto dec = Noise::Encryptor( Noise::Detail::CipherState(s)
					   , Noise::Detail::CipherState(r)
					   , ck
					   );

		auto m_string = std::string("hello");
		auto m = std::vector<std::uint8_t>(m_string.begin(), m_string.end());

		for (auto i = 0; i < 1002; ++i) {
			auto enc_m = enc.encrypt_message(m);

			auto p_l = dec.decrypt_length(enc_m.first);
			assert(p_l);
			assert(*p_l == m.size());
			auto p_m = dec.decrypt_message(enc_m.second);
			assert(p_m);
			assert(*p_m == m);

			switch(i) {
			case 0:
				assert(enc_m.first == Util::Str::hexread("cf2b30ddf0cf3f80e7c35a6e6730b59fe802"));
				assert(enc_m.second == Util::Str::hexread("473180f396d88a8fb0db8cbcf25d2f214cf9ea1d95"));
				break;
			case 1:
				assert(enc_m.first == Util::Str::hexread("72887022101f0b6753e0c7de21657d35a4cb"));
				assert(enc_m.second == Util::Str::hexread("2a1f5cde2650528bbc8f837d0f0d7ad833b1a256a1"));
				break;
			case 500:
				assert(enc_m.first == Util::Str::hexread("178cb9d7387190fa34db9c2d50027d21793c"));
				assert(enc_m.second == Util::Str::hexread("9bc2d40b1e14dcf30ebeeeb220f48364f7a4c68bf8"));
				break;
			case 501:
				assert(enc_m.first == Util::Str::hexread("1b186c57d44eb6de4c057c49940d79bb838a"));
				assert(enc_m.second == Util::Str::hexread("145cb528d6e8fd26dbe50a60ca2c104b56b60e45bd"));
				break;
			case 1000:
				assert(enc_m.first == Util::Str::hexread("4a2f3cc3b5e78ddb83dcb426d9863d9d9a72"));
				assert(enc_m.second == Util::Str::hexread("3b0337c89dd0b005d89f8d3c05c52b76b29b740f09"));
				break;
			case 1001:
				assert(enc_m.first == Util::Str::hexread("2ecd8c8a5629d0d02ab457a0fdd0f7b90a19"));
				assert(enc_m.second == Util::Str::hexread("2cd46be5ecb6ca570bfc5e268338b1a16cf4ef2d36"));
			default: continue;
			}
		}
	}

	return 0;
}

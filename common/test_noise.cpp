#include<assert.h>
#include"Crypto/Secret.hpp"
#include"Noise/Detail/hkdf2.hpp"

int main() {
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

	return 0;
}

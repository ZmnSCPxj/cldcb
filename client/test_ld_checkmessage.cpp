#include<assert.h>
#include"LD/checkmessage.hpp"
#include"Secp256k1/PubKey.hpp"
#include"Secp256k1/Signature.hpp"

int main() {

	/* @bitconner */
	{
		auto res = LD::checkmessage
			( Secp256k1::PubKey("02b80cabdf82638aac86948e4c06e82064f547768dcef977677b9ea931ea75bab5")
			/* Not in zbase32, but in hex, because raisins.   */
			, Secp256k1::Signature("4c5ac1329697cd1c67419cf3c3ee7ba9a2af883a42846023ac1e24a8f7cc64bc4b93916b6a370a34d94a5e90f40a0614a1b3a7670eab9983cd485237be7e13f1")
			, "is this compatible?"
			);
		assert(res);
	}
	/* @duck1123 */
	{
		auto res = LD::checkmessage
			( Secp256k1::PubKey("02de60d194e1ca5947b59fe8e2efd6aadeabfb67f2e89e13ae1a799c1e08e4a43b")
			, Secp256k1::Signature("88de3136a4ec1c31225ad6400b9f6baaded81b63fdfdf3cb2cfdaae19e72d8bb0cef1580cbd16d737f6e3951e679a4ad4603a7bbeba928a4ee99af8e1cd7e9e6")
			, "hi"
			);
		assert(res);
	}

	return 0;
}


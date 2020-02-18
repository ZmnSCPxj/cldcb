#include<assert.h>
#include"Sha256/Hash.hpp"
#include"Sha256/fun.hpp"
#include"Sha256/hmac.hpp"
#include"Util/Str.hpp"

int main () {
	/* Check comparison.  */
	assert( Sha256::Hash("da978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb")
	     != Sha256::Hash("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb")
	      );
	assert( Sha256::Hash("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb")
	     == Sha256::Hash("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb")
	      );

	/* Check buffer use.  */
	{
		std::uint8_t buffer[32];

		auto h1 = Sha256::Hash("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb");
		h1.to_buffer(buffer);
		auto h2 = Sha256::Hash::from_buffer(buffer);
		assert(h1 == h2);
	}

	/* From sha256sum program.  */
	assert(Sha256::fun("a", 1) == Sha256::Hash("ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb"));
	assert(Sha256::fun("aa", 2) == Sha256::Hash("961b6dd3ede3cb8ecbaacbd68de040cd78eb2ed5889130cceb4c49268ea4d506"));
	assert(Sha256::fun("ZmnSCPxj", 8) == Sha256::Hash("d83ef74d099b845cf9851baa7fba19b43c3bfeca0b7b709d150c49058948a799"));

	/* From Wikipedia.  */
	{
		auto msg = std::string("The quick brown fox jumps over the lazy dog");
		assert( Sha256::hmac("key", 3, &msg[0], msg.length())
		     == Sha256::Hash("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8")
		      );
	}

	auto hmactest = []( std::string const& hexkey
			  , std::string const& hexdata
			  , std::string const& hexhash
			  ) {
		auto key = Util::Str::hexread(hexkey);
		auto data = Util::Str::hexread(hexdata);
		assert( Sha256::hmac( &key[0], key.size()
				    , &data[0], data.size()
				    )
		     == Sha256::Hash(hexhash)
		      );
	};
	/* From RFC 4231.  */
	/* 1 */
	hmactest( "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b"
		, "4869205468657265"
		, "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"
		);
	/* 2 */
	hmactest( "4a656665"
		, "7768617420646f2079612077616e7420666f72206e6f7468696e673f"
		, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"
		);
	/* 3 */
	hmactest( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		, "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
		, "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe"
		);
	/* 6 */
	hmactest( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		, "54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a65204b6579202d2048617368204b6579204669727374"
		, "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"
		);
	/* 7 */
	hmactest( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		, "5468697320697320612074657374207573696e672061206c6172676572207468616e20626c6f636b2d73697a65206b657920616e642061206c6172676572207468616e20626c6f636b2d73697a6520646174612e20546865206b6579206e6565647320746f20626520686173686564206265666f7265206265696e6720757365642062792074686520484d414320616c676f726974686d2e"
		, "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2"
		);

	return 0;
}

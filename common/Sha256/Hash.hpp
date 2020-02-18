#ifndef CLDCB_COMMON_SHA256_HASH_HPP
#define CLDCB_COMMON_SHA256_HASH_HPP

#include<cstdint>
#include<ostream>

namespace Sha256 {

class Hash {
private:
	std::uint8_t hash[32];
public:
	Hash() {
		for (auto i = 0; i < 32; ++i)
			hash[i] = 0;
	}
	Hash(std::string const&);
	Hash(Hash const&) =default;
	Hash(Hash&&) =default;
	Hash& operator=(Hash const&) =default;
	Hash& operator=(Hash&&) =default;

	bool operator==(Hash const& o) const {
		for (auto i = 0; i < 32; ++i)
			if (hash[i] != o.hash[i])
				return false;
		return true;
	}
	bool operator!=(Hash const& o) const {
		return !(*this == o);
	}

	static Hash from_buffer(std::uint8_t const buffer[32]) {
		auto ret = Hash();
		for (auto i = 0; i < 32; ++i)
			ret.hash[i] = buffer[i];
		return ret;
	}
	void to_buffer(std::uint8_t buffer[32]) const {
		for (auto i = 0; i < 32; ++i)
			buffer[i] = hash[i];
	}
};

}

std::ostream& operator<<(std::ostream&, Sha256::Hash const&);

#endif /* CLDCB_COMMON_SHA256_HASH_HPP */

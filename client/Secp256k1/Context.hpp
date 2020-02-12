#ifndef CLDCB_CLIENT_SECP256K1_CONTEXT_HPP
#define CLDCB_CLIENT_SECP256K1_CONTEXT_HPP

#include<memory>

extern "C" {
struct secp256k1_context_struct;
}

namespace Secp256k1 {

class Context {
private:
	std::shared_ptr<secp256k1_context_struct> pimpl;
public:
	static const int Verify = 1 << 0;
	static const int Sign = 1 << 1;

	explicit Context(int flags = Verify | Sign);
	Context(Context&&);
	Context(Context const&) =default;

	Context& operator=(Context&&) =default;
	Context& operator=(Context const&) =default;

	explicit operator bool() const { return !!pimpl; }
	bool operator!() const { return !pimpl; }

	secp256k1_context_struct const* get() const { return pimpl.get(); }
	secp256k1_context_struct* get() { return pimpl.get(); }
};

}

#endif /* CLDCB_CLIENT_SECP256K1_CONTEXT_HPP */

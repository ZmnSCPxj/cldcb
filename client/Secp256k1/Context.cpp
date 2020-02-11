#include<secp256k1.h>
#include<stdexcept>
#include<string>

#include"Secp256k1/Context.hpp"

namespace {

/* Called due to illegal inputs to the library.  */
void illegal_callback(const char* msg, void*) {
	throw std::invalid_argument(std::string("SECP256K1: ") + msg);
}

}

namespace Secp256k1 {

Context::Context(int flags) {
	int c_flags = SECP256K1_CONTEXT_NONE;
	if (flags & Sign)
		c_flags |= SECP256K1_CONTEXT_SIGN;
	if (flags & Verify)
		c_flags |= SECP256K1_CONTEXT_VERIFY;

	auto ctx = secp256k1_context_create(c_flags);
	pimpl = (void*) ctx;
	secp256k1_context_set_illegal_callback( ctx
					      , &illegal_callback
					      , nullptr
					      );
}

Context::Context(Context&& o) {
	pimpl = o.pimpl;
	o.pimpl = nullptr;
}

Context::Context(Context const& o) {
	auto ctx = secp256k1_context_clone((secp256k1_context *) o.pimpl);
	pimpl = (void*) ctx;
}

Context::~Context() {
	secp256k1_context_destroy((secp256k1_context*) pimpl);
}

}

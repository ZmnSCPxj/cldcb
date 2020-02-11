#ifndef CLDCB_CLIENT_SECP256K1_CONTEXT_HPP
#define CLDCB_CLIENT_SECP256K1_CONTEXT_HPP

namespace Secp256k1 {

class Context {
private:
	void *pimpl;
public:
	static const int Verify = 1 << 0;
	static const int Sign = 1 << 1;

	explicit Context(int flags = Verify | Sign);
	Context(nullptr_t) : pimpl(nullptr) { }
	Context(Context&&);
	Context(Context const&);

	~Context();

	Context& operator=(Context&&) =default;
	Context& operator=(Context const&) =default;

	explicit operator bool() const { return !!pimpl; }
	bool operator!() const { return !pimpl; }

	void const* get() const { return pimpl; }
	void* get() { return pimpl; }
};

}

#endif /* CLDCB_CLIENT_SECP256K1_CONTEXT_HPP */

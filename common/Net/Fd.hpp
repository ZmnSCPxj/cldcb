#ifndef CLDCB_COMMON_NET_FD_HPP
#define CLDCB_COMMON_NET_FD_HPP

namespace Net {

/* RAII class for a file descriptor.  */
class Fd {
private:
	int fd;

public:
	Fd() : fd(-1) { }
	explicit Fd(int fd_) : fd(fd_) { }
	Fd(Fd const&) =delete;
	Fd(Fd&& o) : fd(o.release()) { }

	Fd& operator=(Fd&& o);

	~Fd();

	int get() const { return fd; }
	int release() {
		auto ret = fd;
		fd = -1;
		return ret;
	}
	void swap(Fd& o) {
		auto tmp = o.fd;
		o.fd = fd;
		fd = tmp;
	}
	void reset(int fd_) {
		*this = Fd(fd_);
	}

	operator bool() const {
		return (fd >= 0);
	}
	bool operator!() const {
		return !((bool) *this);
	}
};

}

#endif /* CLDCB_COMMON_NET_FD_HPP */

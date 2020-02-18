#ifndef CLDCB_CLIENT_LD_WRITER_HPP
#define CLDCB_CLIENT_LD_WRITER_HPP

#include<memory>
#include<ostream>
#include<string>

namespace LD {

/* Intended to be the single object that outputs to the given
 * output stream; other objects that need to write to the
 * given output stream must do it via this object.
 * Other objects can safely call write() on this object from
 * any thread (and indeed that is the whole point: a string
 * from one thread is printed completely before a string
 * from another thread is serviced).
 */
class Writer {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	~Writer();
	explicit Writer(std::ostream&);
	Writer() =delete;
	Writer(Writer&&);
	Writer(Writer const&) =delete;

	void write(std::string const&);
};

}

#endif /* CLDCB_CLIENT_LD_WRITER_HPP */

#ifndef CLDCB_SERVER_ARCHIVE_UNLINKER_HPP
#define CLDCB_SERVER_ARCHIVE_UNLINKER_HPP

#include<string>

namespace Archive {

/* A kickable RAII class which automatically unlinks the
 * specified file on destruction, unless kicked by
 * do_not_unlink().
 */
class Unlinker {
private:
	std::string filename;
	bool to_unlink;

public:
	Unlinker() =delete;
	Unlinker(Unlinker&& o) {
		filename = std::move(o.filename);
		to_unlink = o.to_unlink;
		o.to_unlink = false;
	}
	Unlinker& operator=(Unlinker&&) =default;
	Unlinker(Unlinker const&) =delete;
	/* RAII.  */
	~Unlinker();

	explicit Unlinker( std::string filename_
			 ) : filename(std::move(filename_))
			   , to_unlink(true)
			   { }

	void do_not_unlink() {
		filename = "";
		to_unlink = false;
	}
};

}

#endif /* CLDCB_SERVER_ARCHIVE_UNLINKER_HPP */

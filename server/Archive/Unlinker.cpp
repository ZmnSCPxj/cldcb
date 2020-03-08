#include<errno.h>
#include<unistd.h>
#include"Archive/Unlinker.hpp"

namespace Archive {

Unlinker::~Unlinker() {
	if (!to_unlink)
		return;
	(void) unlink(filename.c_str());
	errno = 0;
}

}


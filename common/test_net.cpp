#include<assert.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include"Net/DirectConnector.hpp"
#include"Net/Fd.hpp"
#include"Net/SocketFd.hpp"

int main() {

	{
		auto fd = Net::Fd(-1);
		assert(!fd);
		assert(!((bool) fd));
		auto sfd = Net::SocketFd(std::move(fd));
		assert(!fd);
		assert(!sfd);
		assert(!((bool)sfd));
	}

	/* Only works on nixlikes.  */
	{
		auto fd = Net::Fd(open("/dev/null", O_RDONLY));
		assert(fd);
		fd = std::move(fd);
		assert(fd);
		auto sfd = Net::SocketFd(std::move(fd));
		assert(!fd);
		assert(sfd);
		sfd = std::move(sfd);
		assert(sfd);
		close(sfd.release());
		assert(!sfd);
	}

	/* Only works if you have normal network access... */
	if (1) /* Set to 0 if you do not have normal network access.  */
	{
		auto connector = Net::DirectConnector();
		auto sfd = connector.connect("www.google.com", 80);
		assert(sfd);
	}

	return 0;
}

#include<assert.h>
#include<cstdint>
#include<fcntl.h>
#include<future>
#include<sys/stat.h>
#include<sys/types.h>
#include<thread>
#include<unistd.h>
#include<vector>
#include"Net/DirectConnector.hpp"
#include"Net/Fd.hpp"
#include"Net/Listener.hpp"
#include"Net/SocketFd.hpp"
#include"Secp256k1/Random.hpp"
#include"Util/Logger.hpp"

class NullLogger : public Util::Logger {
public:
	void log(LogLevel, std::string) override { }
};

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

	/* Only works if the port given below is allowed by your
	 * firewall, and you are not already having a listener
	 * on that port.
	 */
	if (1) /* Set to 0 if your firewall disallows or otherwise cannot use the port.  */
	{
		auto port = 29736;
		auto len = 6;

		Secp256k1::Random rand;
		/* We need to wait for the server to establish
		 * the listener before we can connect to it.
		 * This promise synchronizes the server and the
		 * client threads.
		 */
		auto promise = std::promise<void>();

		auto data = ([&rand, len]() {
			auto data = std::vector<std::uint8_t>();
			for (auto i = 0; i < len; ++i)
				data.push_back(rand.get());
			return data;
		})();
		auto server = std::thread([&promise, port, len]() {
			auto logger = NullLogger();
			auto listener = Net::Listener(port, logger);
			promise.set_value();
			auto sock = listener.accept();
			assert(sock);
			auto data = std::vector<std::uint8_t>(len);
			auto rres = ::read(sock.get(), &data[0], len);
			assert(rres == ssize_t(len));
			auto wres = ::write(sock.get(), &data[0], len);
			assert(wres == ssize_t(len));
		});
		auto client = std::thread([&promise, &data, port, len]() {
			auto connector = Net::DirectConnector();
			promise.get_future().get();
			auto sock = connector.connect("127.0.0.1", port);
			assert(sock);
			auto wres = ::write(sock.get(), &data[0], len);
			assert(wres == ssize_t(len));
			auto ret_data = std::vector<std::uint8_t>(len);
			auto rres = ::read(sock.get(), &ret_data[0], len);
			assert(rres == ssize_t(len));
			assert(data == ret_data);
		});
		server.join();
		client.join();
	}

	return 0;
}

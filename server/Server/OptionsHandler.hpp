#ifndef CLDCB_SERVER_SERVER_OPTIONSHANDLER_HPP
#define CLDCB_SERVER_SERVER_OPTIONSHANDLER_HPP

#include<cstdint>
#include<memory>
#include<string>
#include<vector>

namespace Util { class Logger; }

namespace Server {

class OptionsHandler {
private:
	class Impl;
	std::unique_ptr<Impl> pimpl;

public:
	OptionsHandler() =delete;

	OptionsHandler(OptionsHandler&&);
	OptionsHandler& operator=(OptionsHandler&& o) =default;

	~OptionsHandler();

	explicit
	OptionsHandler(Util::Logger& logger);

	/* Also changes current directory to the server-dir.
	 * If it returns nullptr, continue as normal.
	 * If it returns non-null, exit with the given code.
	 * `-` and `--` options are stripped and parsed, leaving
	 * only the parameters.
	 */
	std::unique_ptr<int> handle(std::vector<std::string>&);

	std::string const& pidfile() const;

	std::string const& logfile() const;
	std::uint16_t max_count() const;
	int port() const;
};

}

#endif /* CLDCB_SERVER_SERVER_OPTIONSHANDLER_HPP */

#include<assert.h>
#include<utility>
#include<vector>
#include"Util/Logger.hpp"

class LoggerTest : public Util::Logger {
private:
	std::vector<std::pair<Util::Logger::LogLevel, std::string>> logs;

public:
	LoggerTest() =default;

	std::vector<std::pair<Util::Logger::LogLevel, std::string>> const&
	get_logs() const {
		return logs;
	}

	void log(Util::Logger::LogLevel l, std::string s) override {
		logs.push_back(std::make_pair(l, s));
	}
};

int main() {
	auto log = LoggerTest();

	log.debug("debug-level log %d", 0);
	assert(log.get_logs().size() == 1);
	assert(log.get_logs()[0].first == Util::Logger::Debug);
	assert(log.get_logs()[0].second == "debug-level log 0");

	log.info("%s-level log", "info");
	assert(log.get_logs().size() == 2);
	assert(log.get_logs()[1].first == Util::Logger::Info);
	assert(log.get_logs()[1].second == "info-level log");

	log.unusual("unusual-level log");
	assert(log.get_logs().size() == 3);
	assert(log.get_logs()[2].first == Util::Logger::Unusual);
	assert(log.get_logs()[2].second == "unusual-level log");

	log.BROKEN("oh no");
	assert(log.get_logs().size() == 4);
	assert(log.get_logs()[3].first == Util::Logger::Broken);
	assert(log.get_logs()[3].second == "oh no");

	return 0;
}

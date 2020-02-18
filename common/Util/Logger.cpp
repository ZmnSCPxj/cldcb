#include<assert.h>
#include<cstdio>
#include<cstdarg>
#include<vector>
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace {

std::string format(char const *fmt, va_list va) {
	auto buffer = std::vector<char>(9);
	auto real_size = 0;
	auto ok = false;

	for(;;) {
		va_list va2;

		va_copy(va2, va);
		real_size = std::vsnprintf( &buffer[0], buffer.size()
					  , fmt, va2
					  );
		va_end(va2);

		if (real_size >= buffer.size())
			buffer.resize(real_size + 1);
		else
			break;
	}

	return std::string(&buffer[0]);
}

}

namespace Util {

void Logger::debug(char const *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto s = format(fmt, va);
	va_end(va);
	log(Debug, s);
}
void Logger::info(char const *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto s = format(fmt, va);
	va_end(va);
	log(Info, s);
}
void Logger::unusual(char const *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto s = format(fmt, va);
	va_end(va);
	log(Unusual, s);
}
void Logger::BROKEN(char const *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	auto s = format(fmt, va);
	va_end(va);
	log(Broken, s);
}

}

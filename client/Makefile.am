bin_PROGRAMS = cldcb-plugin

cldcb_plugin_SOURCES = \
	LD/Logger.cpp \
	LD/Logger.hpp \
	LD/Writer.cpp \
	LD/Writer.hpp \
	Plugin/Main.cpp \
	Plugin/Main.hpp \
	plugin.cpp

AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = ../common/libcldcb-common.la $(PTHREAD_LIBS)
AM_CXXFLAGS = -I../common/ $(PTHREAD_CFLAGS)

TESTS = \
	test_ld_logger \
	test_ld_writer
check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

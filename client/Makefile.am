bin_PROGRAMS = cldcb-plugin

# Programs are just a single cpp file.
cldcb_plugin_SOURCES = \
	plugin.cpp

# The rest of the program is in a convenience library,
# to facilitate testing.
# A program is a library.
noinst_LTLIBRARIES = libcldcb-client.la
libcldcb_client_la_SOURCES = \
	LD/DbWrite.hpp \
	LD/Logger.cpp \
	LD/Logger.hpp \
	LD/Writer.cpp \
	LD/Writer.hpp \
	Plugin/DbFileReader.cpp \
	Plugin/DbFileReader.hpp \
	Plugin/DbWriteHandler.cpp \
	Plugin/DbWriteHandler.hpp \
	Plugin/Main.cpp \
	Plugin/Main.hpp \
	Plugin/MainLoop.cpp \
	Plugin/MainLoop.hpp \
	Plugin/OptFile/load.cpp \
	Plugin/OptFile/load.hpp \
	Plugin/OptFile/parse.cpp \
	Plugin/OptFile/parse.hpp \
	Plugin/OptFile/translate_setup.cpp \
	Plugin/OptFile/translate_setup.hpp \
	Plugin/OptFile/validate_keys.cpp \
	Plugin/OptFile/validate_keys.hpp \
	Plugin/OptHandler.cpp \
	Plugin/OptHandler.hpp \
	Plugin/ServerIf.hpp \
	Plugin/ServerIncrementIf.hpp \
	Plugin/ServerResult.hpp \
	Plugin/ServerReuploadIf.hpp \
	Plugin/ServerSpec.hpp \
	Plugin/Setup.hpp
libcldcb_client_la_LIBADD = ../common/libcldcb-common.la

AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = libcldcb-client.la $(PTHREAD_LIBS)
AM_CXXFLAGS = -I$(top_srcdir)/common/ $(PTHREAD_CFLAGS)

TESTS = \
	test_ld_logger \
	test_ld_writer \
	test_optfile
check_PROGRAMS = $(TESTS)

EXTRA_DIST = optfile-example

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND


AM_CXXFLAGS = \
	-I$(top_srcdir)/client/ \
	-I$(top_srcdir)/common/ \
	-I$(top_srcdir)/server/ \
	$(PTHREAD_CFLAGS)
LDADD = \
	../client/libcldcb-client.la \
	../server/libcldcb-server.la \
	$(PTHREAD_LIBS)

AM_DEFAULT_SOURCE_EXT = .cpp

TESTS =

check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

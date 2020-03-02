bin_PROGRAMS = cldcb-server

cldcb_server_SOURCES = \
	Daemon/AcceptHandler.cpp \
	Daemon/AcceptHandler.hpp \
	Daemon/AcceptLoop.cpp \
	Daemon/AcceptLoop.hpp \
	Daemon/Breaker.cpp \
	Daemon/Breaker.hpp \
	Daemon/ClientList.cpp \
	Daemon/ClientList.hpp \
	Daemon/Connection.cpp \
	Daemon/Connection.hpp \
	Daemon/ConnectionHandshaker.cpp \
	Daemon/ConnectionHandshaker.hpp \
	Daemon/IoResult.hpp \
	Daemon/KeyKeeper.cpp \
	Daemon/KeyKeeper.hpp \
	Daemon/Main.cpp \
	Daemon/Main.hpp \
	Daemon/PidFiler.cpp \
	Daemon/PidFiler.hpp \
	Ev/Io.hpp \
	Ev/ThreadPool.cpp \
	Ev/ThreadPool.hpp \
	Ev/concurrent.cpp \
	Ev/concurrent.hpp \
	Ev/now.cpp \
	Ev/now.hpp \
	Ev/start.cpp \
	Ev/start.hpp \
	Ev/wait_io.cpp \
	Ev/wait_io.hpp \
	Ev/yield.cpp \
	Ev/yield.hpp \
	Server/AddRemove.cpp \
	Server/AddRemove.hpp \
	Server/Daemon.cpp \
	Server/Daemon.hpp \
	Server/Logger.cpp \
	Server/Logger.hpp \
	Server/Stop.cpp \
	Server/Stop.hpp \
	Server/TermLogger.cpp \
	Server/TermLogger.hpp \
	Server/change_clients.cpp \
	Server/change_clients.hpp \
	Server/send_signal.cpp \
	Server/send_signal.hpp \
	daemonize.cpp \
	daemonize.hpp \
	server.cpp

AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = ../common/libcldcb-common.la $(PTHREAD_LIBS) $(libev_LIBS)
AM_CXXFLAGS = -I$(top_srcdir)/common/ $(PTHREAD_CFLAGS) $(libev_CFLAGS)

TESTS =
check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

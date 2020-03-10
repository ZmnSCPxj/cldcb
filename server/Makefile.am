bin_PROGRAMS = cldcb-server
cldcb_server_SOURCES = \
	server.cpp

noinst_LTLIBRARIES = libcldcb-server.la
libcldcb_server_la_SOURCES = \
	Archive/FooterJudge.cpp \
	Archive/FooterJudge.hpp \
	Archive/IncrementalAppend.cpp \
	Archive/IncrementalAppend.hpp \
	Archive/IncrementalOnlySequence.hpp \
	Archive/IncrementalThenReuploadSequence.cpp \
	Archive/IncrementalThenReuploadSequence.hpp \
	Archive/IncrementalWriter.cpp \
	Archive/IncrementalWriter.hpp \
	Archive/ReuploadSequence.cpp \
	Archive/ReuploadSequence.hpp \
	Archive/ReuploadWriter.cpp \
	Archive/ReuploadWriter.hpp \
	Archive/Unlinker.cpp \
	Archive/Unlinker.hpp \
	Backup/ConnectionLoop.cpp \
	Backup/ConnectionLoop.hpp \
	Backup/DataReader.hpp \
	Backup/DataStorage.cpp \
	Backup/DataStorage.hpp \
	Backup/IncrementalStorage.hpp \
	Backup/RecognitionStorage.hpp \
	Backup/RequestIncremental.cpp \
	Backup/RequestIncremental.hpp \
	Backup/ReuploadStorage.hpp \
	Backup/ServiceLoop.cpp \
	Backup/ServiceLoop.hpp \
	Backup/StorageIf.hpp \
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
	Daemon/ConnectionLoop.hpp \
	Daemon/IoResult.hpp \
	Daemon/KeyKeeper.cpp \
	Daemon/KeyKeeper.hpp \
	Daemon/Main.cpp \
	Daemon/Main.hpp \
	Daemon/Messenger.cpp \
	Daemon/Messenger.hpp \
	Daemon/PidFiler.cpp \
	Daemon/PidFiler.hpp \
	Ev/Io.hpp \
	Ev/Join.cpp \
	Ev/Join.hpp \
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
	daemonize.hpp
libcldcb_server_la_LIBADD = ../common/libcldcb-common.la

AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = libcldcb-server.la $(PTHREAD_LIBS) $(libev_LIBS)
AM_CXXFLAGS = -I$(top_srcdir)/common/ $(PTHREAD_CFLAGS) $(libev_CFLAGS)

TESTS = \
	test_archive_abort
check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

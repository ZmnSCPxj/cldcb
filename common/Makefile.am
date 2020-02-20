LIBSECP256K1_LIBRARY = 
LIBSECP256K1_INCLUDE = -I../external/secp256k1/include/

noinst_LTLIBRARIES = libcldcb-common.la
libcldcb_common_la_SOURCES = \
	Jsmn/Detail/EndAdvancer.cpp \
	Jsmn/Detail/EndAdvancer.hpp \
	Jsmn/Detail/ParseResult.hpp \
	Jsmn/Detail/Str.cpp \
	Jsmn/Detail/Str.hpp \
	Jsmn/Detail/Token.cpp \
	Jsmn/Detail/Token.hpp \
	Jsmn/Detail/Type.hpp \
	Jsmn/Object.cpp \
	Jsmn/Object.hpp \
	Jsmn/Parser.cpp \
	Jsmn/Parser.hpp \
	Jsmn/jsonify_string.cpp \
	Jsmn/jsonify_string.hpp \
	S.hpp \
	Secp256k1/Detail/context.cpp \
	Secp256k1/Detail/context.hpp \
	Secp256k1/G.cpp \
	Secp256k1/G.hpp \
	Secp256k1/PrivKey.cpp \
	Secp256k1/PrivKey.hpp \
	Secp256k1/PubKey.cpp \
	Secp256k1/PubKey.hpp \
	Secp256k1/Random.cpp \
	Secp256k1/Random.hpp \
	Secp256k1/ecdh.cpp \
	Secp256k1/ecdh.hpp \
	Sha256/Hash.cpp \
	Sha256/Hash.hpp \
	Sha256/fun.cpp \
	Sha256/fun.hpp \
	Sha256/hmac.cpp \
	Sha256/hmac.hpp \
	Sync/MVar.hpp \
	Util/Logger.cpp \
	Util/Logger.hpp \
	Util/Str.cpp \
	Util/Str.hpp \
	Util/make_unique.hpp

libcldcb_common_la_LIBADD =\
	../external/secp256k1/libsecp256k1.la \
	../external/sha-2/libsha-2.la
libcldcb_common_la_CPPFLAGS = \
	-I../external/secp256k1/include/ \
	-I../external/sha-2/ \
	-I../external/jsmn/

AM_CXXFLAGS = $(PTHREAD_CFLAGS)
LDADD = libcldcb-common.la $(PTHREAD_LIBS)

AM_DEFAULT_SOURCE_EXT = .cpp

TESTS = \
	test_jsmn \
	test_logger \
	test_secp256k1 \
	test_serialize \
	test_sha256 \
	test_sync
check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

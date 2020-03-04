LIBSECP256K1_LIBRARY = 
LIBSECP256K1_INCLUDE = -I../external/secp256k1/include/

noinst_LTLIBRARIES = libcldcb-common.la
libcldcb_common_la_SOURCES = \
	Crypto/Box/Detail/Nonce.hpp \
	Crypto/Box/Sealer.cpp \
	Crypto/Box/Sealer.hpp \
	Crypto/Box/Unsealer.cpp \
	Crypto/Box/Unsealer.hpp \
	Crypto/Secret.cpp \
	Crypto/Secret.hpp \
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
	Main.cpp \
	Main.hpp \
	Net/Connector.hpp \
	Net/Detail/AddrInfoReleaser.cpp \
	Net/Detail/AddrInfoReleaser.hpp \
	Net/DirectConnector.cpp \
	Net/DirectConnector.hpp \
	Net/Fd.cpp \
	Net/Fd.hpp \
	Net/Listener.cpp \
	Net/Listener.hpp \
	Net/SocketFd.cpp \
	Net/SocketFd.hpp \
	Noise/Detail/Aead.cpp \
	Noise/Detail/Aead.hpp \
	Noise/Detail/CipherState.cpp \
	Noise/Detail/CipherState.hpp \
	Noise/Detail/HS.cpp \
	Noise/Detail/HS.hpp \
	Noise/Detail/hkdf2.cpp \
	Noise/Detail/hkdf2.hpp \
	Noise/Encryptor.cpp \
	Noise/Encryptor.hpp \
	Noise/Initiator.cpp \
	Noise/Initiator.hpp \
	Noise/Responder.cpp \
	Noise/Responder.hpp \
	Protocol/MID.hpp \
	Protocol/Message.hpp \
	S.hpp \
	Secp256k1/Detail/context.cpp \
	Secp256k1/Detail/context.hpp \
	Secp256k1/G.cpp \
	Secp256k1/G.hpp \
	Secp256k1/KeyPair.hpp \
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
	Sha256/hkdf.cpp \
	Sha256/hkdf.hpp \
	Sha256/hmac.cpp \
	Sha256/hmac.hpp \
	Stream/Sink.hpp \
	Stream/SinkSource.hpp \
	Stream/Source.hpp \
	Stream/read_line.cpp \
	Stream/read_line.hpp \
	Sync/MVar.hpp \
	Util/Logger.cpp \
	Util/Logger.hpp \
	Util/Str.cpp \
	Util/Str.hpp \
	Util/make_unique.hpp

libcldcb_common_la_LIBADD =\
	../external/secp256k1/libsecp256k1.la \
	../external/libsodium/src/libsodium/libsodium.la
libcldcb_common_la_CPPFLAGS = \
	-I$(top_srcdir)/external/secp256k1/include/ \
	-I$(top_srcdir)/external/libsodium/src/libsodium/include/ \
	-I$(top_srcdir)/external/jsmn/

AM_CXXFLAGS = $(PTHREAD_CFLAGS)
LDADD = libcldcb-common.la $(PTHREAD_LIBS)

AM_DEFAULT_SOURCE_EXT = .cpp

TESTS = \
	test_crypto_box \
	test_jsmn \
	test_logger \
	test_net \
	test_noise \
	test_read_line \
	test_secp256k1 \
	test_secret \
	test_serialize \
	test_sha256 \
	test_sync
check_PROGRAMS = $(TESTS)

if USE_VALGRIND
LOG_COMPILER = valgrind
AM_LOG_FLAGS = --leak-check=full --error-exitcode=1
endif USE_VALGRIND

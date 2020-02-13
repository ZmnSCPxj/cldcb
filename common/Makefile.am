LIBSECP256K1_LIBRARY = ../external/secp256k1/libsecp256k1.la
LIBSECP256K1_INCLUDE = -I../external/secp256k1/include/

noinst_LTLIBRARIES = libcldcb-common.la
libcldcb_common_la_SOURCES = \
	Secp256k1/Context.cpp \
	Secp256k1/Context.hpp \
	Secp256k1/G.cpp \
	Secp256k1/G.hpp \
	Secp256k1/PrivKey.cpp \
	Secp256k1/PrivKey.hpp \
	Secp256k1/PubKey.cpp \
	Secp256k1/PubKey.hpp \
	Secp256k1/Random.cpp \
	Secp256k1/Random.hpp

libcldcb_common_la_LIBADD = $(LIBSECP256K1_LIBRARY)
libcldcb_common_la_CPPFLAGS = $(LIBSECP256K1_INCLUDE)


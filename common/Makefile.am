LIBSECP256K1_LIBRARY = 
LIBSECP256K1_INCLUDE = -I../external/secp256k1/include/

noinst_LTLIBRARIES = libcldcb-common.la
libcldcb_common_la_SOURCES = \
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
	Sha256/Hash.cpp \
	Sha256/Hash.hpp \
	Sha256/fun.cpp \
	Sha256/fun.hpp \
	Util/make_unique.hpp

libcldcb_common_la_LIBADD =\
	../external/secp256k1/libsecp256k1.la \
	../external/sha-2/libsha-2.la
libcldcb_common_la_CPPFLAGS = \
	-I../external/secp256k1/include/ \
	-I../external/sha-2/


bin_PROGRAMS = cldcb-plugin

# FIXME: What to do when we are installed separately from the main repo?
LIBSECP256K1_LIBRARY = ../external/secp256k1/libsecp256k1.la
LIBSECP256K1_INCLUDE = -I../external/secp256k1/include/

SECP256K1_WRAPPER_SOURCES = \
	Secp256k1/Context.cpp \
	Secp256k1/Context.hpp \
	Secp256k1/Random.cpp \
	Secp256k1/Random.hpp

cldcb_plugin_SOURCES = \
	$(SECP256K1_WRAPPER_SOURCES) \
	plugin.cpp
cldcb_plugin_LDADD = $(LIBSECP256K1_LIBRARY)
cldcb_plugin_CPPFLAGS = $(LIBSECP256K1_INCLUDE)

ACLOCAL_AMFLAGS = -I m4

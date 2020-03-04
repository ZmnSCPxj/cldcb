
# Subdirectories need to be built
# in this specific order.
SUBDIRS = \
	external/libsodium/ \
	external/secp256k1/ \
	common/ \
	client/ \
	server/ \
	integration/

EXTRA_DIST = \
	external/jsmn/LICENSE \
	external/jsmn/jsmn.h

.PHONY : TAGS

TAGS :
	etags `find -name "*.h" -o -name "*.c" -o -name "*.hpp" -o -name "*.cpp"`

ACLOCAL_AMFLAGS = -I m4

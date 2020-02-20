
SUBDIRS = \
	external/libsodium/ \
	external/sha-2/ \
	external/secp256k1/ \
	common/ \
	client/

EXTRA_DIST = \
	external/jsmn/LICENSE
	external/jsmn/jsmn.h

.PHONY : TAGS

TAGS :
	etags `find -name "*.h" -o -name "*.c" -o -name "*.hpp" -o -name "*.cpp"`

ACLOCAL_AMFLAGS = -I m4

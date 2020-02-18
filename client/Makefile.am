bin_PROGRAMS = cldcb-plugin

cldcb_plugin_SOURCES = \
	LD/Writer.cpp \
	LD/Writer.hpp \
	plugin.cpp
cldcb_plugin_LDADD = ../common/libcldcb-common.la $(PTHREAD_LIBS)
cldcb_plugin_CPPFLAGS = -I../common/
cldcb_plugin_CXXFLAGS = -I../common/ $(PTHREAD_CFLAGS)

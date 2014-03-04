EXTRA_DIST = nscontrol_recovery.sh
bindir = /usr/bin
bin_SCRIPTS = nscontrol_recovery.sh

lib_LTLIBRARIES = libnscontrol.la

libnscontrol_la_SOURCES = nshade_media.cpp nshade_media.h nshade_shared_memory.cpp nshade_shared_memory.h \
nshade_shared_memory_connection.cpp nshade_shared_memory_connection.h nshade_state.cpp nshade_state.h
	
libnscontrol_la_LDFLAGS = -no-undefined -version-info 0:1:0

include_HEADERS = nshade_shared_memory.h nshade_shared_memory_connection.h nshade_media.h nshade_state.h


## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	bsearch.c \
	extern.h \
	hash.c \
	hash.h \
	hash_bigkey.c \
	hash_buf.c \
	hash_func.c \
	hash_log2.c \
	hash_page.c \
	hcreate.c \
	hcreate_r.c \
	page.h \
	qsort.c \
	tdelete.c \
	tdestroy.c \
	tfind.c \
	tsearch.c \
	twalk.c

libsearch_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libsearch.la
libsearch_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

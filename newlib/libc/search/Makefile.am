## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	bsearch.c \
	db_local.h \
	extern.h \
	hash.h \
	page.h \
	qsort.c

## Following are EL/IX level 2 interfaces
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	hash.$(oext) \
	hash_bigkey.$(oext) \
	hash_buf.$(oext) \
	hash_func.$(oext) \
	hash_log2.$(oext) \
	hash_page.$(oext) \
	hcreate.$(oext) \
	hcreate_r.$(oext) \
	tdelete.$(oext) \
	tdestroy.$(oext) \
	tfind.$(oext) \
	tsearch.$(oext) \
	twalk.$(oext)
endif

libsearch_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libsearch.la
libsearch_la_SOURCES = $(GENERAL_SOURCES)
libsearch_la_LIBADD = $(LIB_OBJS)
libsearch_la_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =
endif # USE_LIBTOOL

SUFFIXES = .def

CHEWOUT_FILES =

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)

CLEANFILES = $(CHEWOUT_FILES) *.ref

include $(srcdir)/../../Makefile.shared

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

## All functions in this directory are EL/IX level 2 and above
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	argz_add.$(oext) \
	argz_add_sep.$(oext) \
	argz_append.$(oext) \
	argz_count.$(oext) \
	argz_create.$(oext) \
	argz_create_sep.$(oext) \
	argz_delete.$(oext) \
	argz_extract.$(oext) \
	argz_insert.$(oext) \
	argz_next.$(oext) \
	argz_replace.$(oext) \
	argz_stringify.$(oext) \
	buf_findstr.$(oext) \
	envz_entry.$(oext) \
	envz_get.$(oext) \
	envz_add.$(oext) \
	envz_remove.$(oext) \
	envz_merge.$(oext) \
	envz_strip.$(oext)
endif

libargz_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libargz.la
libargz_la_SOURCES = dummy.c
libargz_la_LIBADD = $(LIB_OBJS)
libargz_la_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = dummy.c
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

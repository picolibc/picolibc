## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

# need a dummy object so objectlist.awk.in will build every time
GENERAL_SOURCES = dummy.c local64.h

## All interfaces are EL/IX level 2
if ELIX_LEVEL_1
LIB_OBJS =
else
LIB_OBJS = \
	fdopen64.$(oext) 	\
	fgetpos64.$(oext)	\
	fopen64.$(oext) 	\
	freopen64.$(oext) 	\
	fseeko64.$(oext) 	\
	fsetpos64.$(oext) 	\
	ftello64.$(oext)	\
	stdio64.$(oext)		\
	tmpfile64.$(oext)
endif

libstdio64_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdio64.la
libstdio64_la_SOURCES = $(GENERAL_SOURCES)
libstdio64_la_LIBADD = $(LIB_OBJS)
libstdio64_la_DEPENDENCIES = $(LIB_OBJS)
LIB_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
LIB_COMPILE = $(COMPILE)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

AM_CFLAGS = -I $(srcdir)/../stdio

CHEWOUT_FILES = \
	fgetpos64.def		\
	fopen64.def		\
	freopen64.def		\
	fseeko64.def		\
	fsetpos64.def		\
	ftello64.def		\
	tmpfile64.def

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

doc: $(CHEWOUT_FILES)
	cat $(srcdir)/stdio64.tex >> $(TARGETDOC)

CLEANFILES = $(CHEWOUT_FILES) *.ref


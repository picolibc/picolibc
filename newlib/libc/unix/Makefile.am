## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	getcwd.c 	\
	pread.c 	\
	pwrite.c 	\
	sigset.c

## Distinguish different EL/IX level interfaces
ELIX_2_OBJS = \
	ttyname.$(oext)

ELIX_4_OBJS = \
	getlogin.$(oext)	\
	getpass.$(oext) 	\
	getpwent.$(oext) 	\
	getut.$(oext)

if ELIX_LEVEL_1
LIB_OBJS =
else
if ELIX_LEVEL_2
LIB_OBJS = $(ELIX_2_OBJS)
else
if ELIX_LEVEL_3
LIB_OBJS = $(ELIX_2_OBJS)
else
if ELIX_LEVEL_4
LIB_OBJS = $(ELIX_2_OBJS) $(ELIX_4_OBJS)
endif
endif
endif
endif

libunix_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libunix.la
libunix_la_SOURCES = $(GENERAL_SOURCES)
libunix_la_LIBADD = $(LIB_OBJS)
libunix_la_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES =

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

# No doc for unix.
doc:

CLEANFILES = $(CHEWOUT_FILES) *.ref

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	sysclose.c	\
	sysfcntl.c	\
	sysfstat.c	\
	sysgetpid.c	\
	sysgettod.c	\
	syskill.c	\
	syslink.c	\
	syslseek.c	\
	sysopen.c	\
	sysread.c	\
	syssbrk.c	\
	sysstat.c	\
	systimes.c	\
	sysunlink.c	\
	syswrite.c


## Weed out EL/IX level 3 interfaces if necessary
if ELIX_LEVEL_1
LIB_OBJS =
else
if ELIX_LEVEL_2
LIB_OBJS =
else
LIB_OBJS = \
	sysexecve.$(oext)	\
	sysfork.$(oext)		\
	syswait.$(oext)
endif
endif

libsyscalls_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libsyscalls.la
libsyscalls_la_SOURCES = $(GENERAL_SOURCES)
libsyscalls_la_LIBADD = $(LIB_OBJS)
libsyscalls_la_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS)
lib_a_DEPENDENCIES = $(LIB_OBJS)
noinst_DATA =
endif

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES =

SUFFIXES = .def

CHEW = ../../doc/makedoc -f $(srcdir)/../../doc/doc.str

.c.def:
	$(CHEW) < $< > $*.def 2> $*.ref
	touch stmp-def

TARGETDOC = ../tmp.texi

# No doc for syscalls.
doc:

CLEANFILES = $(CHEWOUT_FILES) *.ref

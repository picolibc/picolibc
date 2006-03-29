## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	sysclose.c	\
	sysexecve.c	\
	sysfcntl.c	\
	sysfork.c	\
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
	syswait.c	\
	syswrite.c

libsyscalls_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libsyscalls.la
libsyscalls_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
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

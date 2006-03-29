## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	closedir.c opendir.c readdir.c rewinddir.c scandir.c seekdir.c \
	telldir.c execl.c execle.c execlp.c execv.c execve.c execvp.c \
	popen.c creat.c isatty.c

libposix_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libposix.la
libposix_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
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

# No doc for posix.
doc:

CLEANFILES = $(CHEWOUT_FILES) *.ref

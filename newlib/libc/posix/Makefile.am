## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	closedir.c opendir.c readdir.c rewinddir.c scandir.c seekdir.c \
	telldir.c execl.c execle.c execlp.c execv.c execve.c execvp.c \
	popen.c creat.c isatty.c

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

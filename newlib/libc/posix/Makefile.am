## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	closedir.c creat.c isatty.c \
	opendir.c readdir.c \
	readdir_r.c rewinddir.c telldir.c

ELIX_2_OBJS = \
        scandir.$(oext) seekdir.$(oext)

ELIX_3_OBJS = \
	execl.$(oext) execle.$(oext) execlp.$(oext) execv.$(oext) \
	execve.$(oext) execvp.$(oext)

ELIX_4_OBJS = \
	popen.$(oext) 

if ELIX_LEVEL_1
LIB_OBJS =
else 
if ELIX_LEVEL_2
LIB_OBJS = $(ELIX_2_OBJS)
else 
if ELIX_LEVEL_3
LIB_OBJS = $(ELIX_2_OBJS) $(ELIX_3_OBJS)
else
LIB_OBJS = $(ELIX_2_OBJS) $(ELIX_3_OBJS) $(ELIX_4_OBJS)
endif
endif
endif

libposix_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libposix.la
libposix_la_SOURCES = $(GENERAL_SOURCES)
libposix_la_LIBADD = $(LIB_OBJS)
libposix_la_DEPENDENCIES = $(LIB_OBJS)
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

# No doc for posix.
doc:

CLEANFILES = $(CHEWOUT_FILES) *.ref

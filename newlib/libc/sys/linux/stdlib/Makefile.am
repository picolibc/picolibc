## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	fnmatch.c glob.c regcomp.c regerror.c regex2.h \
	regexec.c regfree.c collate.h collate.c collcmp.c \
	reallocf.c

ELIX_3_OBJS = \
	wordexp.$(oext) \
	wordfree.$(oext)

if ELIX_LEVEL_1
LIB_OBJS =
else
if ELIX_LEVEL_2
LIB_OBJS =
else
LIB_OBJS = $(ELIX_3_OBJS)
endif
endif

libstdlib_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libstdlib.la
libstdlib_la_SOURCES = $(GENERAL_SOURCES)
libstdlib_la_LIBADD = $(LIB_OBJS) 
libstdlib_la_DEPENDENCIES = $(LIB_OBJS) 
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(LIB_OBJS) 
lib_a_DEPENDENCIES = $(LIB_OBJS) 
noinst_DATA =
endif # USE_LIBTOOL

AM_CFLAGS = -D_GNU_SOURCE

include $(srcdir)/../../../../Makefile.shared

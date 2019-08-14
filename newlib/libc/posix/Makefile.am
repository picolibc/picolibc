## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

GENERAL_SOURCES = \
	collate.c collcmp.c \
	regcomp.c regerror.c regexec.c regfree.c

ELIX_2_SOURCES =

ELIX_3_SOURCES =

ELIX_4_SOURCES =

if ELIX_LEVEL_1
ELIX_SOURCES =
else 
if ELIX_LEVEL_2
ELIX_SOURCES = $(ELIX_2_SOURCES)
else 
if ELIX_LEVEL_3
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES)
else
ELIX_SOURCES = $(ELIX_2_SOURCES) $(ELIX_3_SOURCES) $(ELIX_4_SOURCES)
endif
endif
endif

libposix_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libposix.la
libposix_la_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(GENERAL_SOURCES) $(ELIX_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = \
	popen.def \
	posix_spawn.def

CHAPTERS = posix.tex

AM_CFLAGS = -D_GNU_SOURCE

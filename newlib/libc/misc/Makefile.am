## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = __dprintf.c unctrl.c ffs.c init.c fini.c

if NEWLIB_RETARGETABLE_LOCKING
LIB_SOURCES += \
	lock.c
endif

libmisc_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libmisc.la
libmisc_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA = 
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = unctrl.def lock.def ffs.def
CHAPTERS = misc.tex

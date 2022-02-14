## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES =
lib_a_CFLAGS = $(AM_CFLAGS)

# This is a hack to force automake to include a definition for
# COMPILE.
EXTRA_LIBRARIES = libfoo.a
libfoo_a_SOURCES = crt0.c

all-local: crt0.o

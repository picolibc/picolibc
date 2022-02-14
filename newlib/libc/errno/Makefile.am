## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = errno.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)

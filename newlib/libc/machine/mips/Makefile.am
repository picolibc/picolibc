## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S strlen.c strcmp.c strncpy.c memset.S memcpy.S
lib_a_CCASFLAGS=$(AM_CCASFLAGS) -D_COMPILING_NEWLIB
lib_a_CFLAGS=$(AM_CFLAGS) -D_COMPILING_NEWLIB

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

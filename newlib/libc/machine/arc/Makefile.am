## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S memcpy.S memcpy-700.S strcpy.c strcpy-600.S \
  strcpy-700.S strcmp-a4.c strcmp.S strchr-a4.c strchr.S strchr-700.S \
  memset-a4.c memset.S strlen-a4.c strlen.S strlen-600.S memcmp.S \
  strncpy.S
lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES =

# This is a hack to force automake to include a definition for
# COMPILE.
EXTRA_LIBRARIES = libfoo.a
libfoo_a_SOURCES = crt0.c

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

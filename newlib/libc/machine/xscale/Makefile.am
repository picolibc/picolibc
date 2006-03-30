## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CCASFLAGS) $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = \
	memchr.c memcmp.c memcpy.c memmove.c memset.c \
	strchr.c strcmp.c strcpy.c strlen.c setjmp.S

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

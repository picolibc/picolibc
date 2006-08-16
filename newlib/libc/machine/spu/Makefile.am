## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

AM_CCASFLAGS = $(INCLUDES)

lib_a_SOURCES = setjmp.S memcpy.c

lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../.. 
AM_CFLAGS = -I $(srcdir)/../../stdio -I $(srcdir)/../../stdlib
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host


## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = syscall.S

AM_CFLAGS = -I$(srcdir)/../..
AM_CCASFLAGS = -I$(srcdir)/../.. $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

all-local: crt0.o

ACLOCAL_AMFLAGS = -I ../../../../.. -I ../../../../../.. -I ../../../../../../config
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

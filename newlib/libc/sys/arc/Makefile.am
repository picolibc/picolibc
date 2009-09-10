## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)
CCASCOMPILE = $(CCAS) $(AM_CCASFLAGS) $(CCASFLAGS)
.S.o:
	$(CCASCOMPILE) -c $<

noinst_LIBRARIES = lib.a

lib_a_SOURCES = isatty.c sbrk.c syscalls.c _exit.c unlink_r.c link_r.c
EXTRA_lib_a_SOURCES = dummy.S
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

all: crt0.o

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS) $(CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = syscalls.c trap.S creat.c ftruncate.c truncate.c
lib_a_CFLAGS = $(AM_CFLAGS)

all-local: crt0.o

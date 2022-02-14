## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS) -I$(abs_newlib_basedir)/libc/machine/arm

AM_CCASFLAGS = $(AM_CPPFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = access.c aeabi_atexit.c
if MAY_SUPPLY_SYSCALLS
lib_a_SOURCES += libcfunc.c trap.S syscalls.c
endif
lib_a_CFLAGS = $(AM_CFLAGS)

if MAY_SUPPLY_SYSCALLS
all-local: crt0.o
endif

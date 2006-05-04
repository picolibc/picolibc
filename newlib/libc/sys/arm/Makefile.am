## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

if MAY_SUPPLY_SYSCALLS
extra_objs = libcfunc.o trap.o syscalls.o _exit.o _nmi_isr.o _fault_isr.o
else
extra_objs =
endif

lib_a_SOURCES = aeabi_atexit.c aeabi_memclr.c aeabi_memcpy.c \
		aeabi_memmove.c aeabi_memset.c
lib_a_LIBADD = $(extra_objs)
lib_a_DEPENDENCIES = $(extra_objs)

if MAY_SUPPLY_SYSCALLS
all: crt0.o
endif

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

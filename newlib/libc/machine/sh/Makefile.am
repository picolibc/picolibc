## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

if SH64
lib_a_SOURCES = memcpy.S memset.S setjmp.S strcpy.S strlen.S strcmp.S strncpy.S
else
lib_a_SOURCES = memcpy.S memset.S setjmp.S strcpy.S strlen.S strcmp.S
endif

memcpy.o: asm.h
memset.o: asm.h
setjmp.o: asm.h
strcpy.o: asm.h
strcmp.o: asm.h
strncpy.o: asm.h

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

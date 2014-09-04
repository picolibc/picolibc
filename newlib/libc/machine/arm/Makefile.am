## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S access.c strlen.c strcmp.S strcpy.c \
	        memcpy.S memcpy-stub.c memchr-stub.c memchr.S \
		strlen.c strlen-armv7.S aeabi_memcpy.c \
		aeabi_memcpy-armv7a.S
lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

MEMCPY_DEP=memcpy-armv7a.S memcpy-armv7m.S
STRCMP_DEP=strcmp-arm-tiny.S strcmp-armv4.S strcmp-armv4t.S strcmp-armv6.S \
	strcmp-armv7.S strcmp-armv7m.S

$(lpfx)memcpy.o: $(MEMCPY_DEP)

$(lpfx)memcpy.obj: $(MEMCPY_DEP)

$(lpfx)strcmp.o: $(STRCMP_DEP)

$(lpfx)strcmp.obj: $(STRCMP_DEP)

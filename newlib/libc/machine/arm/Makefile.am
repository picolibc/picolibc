## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S strcmp.S strcpy.c \
		aeabi_memcpy.c aeabi_memcpy-armv7a.S \
		aeabi_memmove.c aeabi_memmove-soft.S \
		aeabi_memset.c aeabi_memset-soft.S aeabi_memclr.c
lib_a_SOURCES += memchr-stub.c
lib_a_SOURCES += memchr.S
lib_a_SOURCES += memcpy-stub.c
lib_a_SOURCES += memcpy.S
lib_a_SOURCES += strlen-stub.c
lib_a_SOURCES += strlen.S

lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

MEMCHR_DEP=acle-compat.h
MEMCPY_DEP=memcpy-armv7a.S memcpy-armv7m.S
STRCMP_DEP=strcmp-arm-tiny.S strcmp-armv4.S strcmp-armv4t.S strcmp-armv6.S \
	strcmp-armv6m.S strcmp-armv7.S strcmp-armv7m.S
AEABI_MEMMOVE_DEP=aeabi_memmove-thumb.S aeabi_memmove-thumb2.S \
	aeabi_memmove-arm.S
AEABI_MEMSET_DEP=aeabi_memset-thumb.S aeabi_memset-thumb2.S \
	aeabi_memset-arm.S

$(lpfx)memchr.o: $(MEMCHR_DEP)
$(lpfx)memchr.obj: $(MEMCHR_DEP)

$(lpfx)memcpy.o: $(MEMCPY_DEP)

$(lpfx)memcpy.obj: $(MEMCPY_DEP)

$(lpfx)strcmp.o: $(STRCMP_DEP)

$(lpfx)strcmp.obj: $(STRCMP_DEP)

$(lpfx)aeabi_memmove.o: $(AEABI_MEMMOVE_DEP)

$(lpfx)aeabi_memmove.obj: $(AEABI_MEMMOVE_DEP)

$(lpfx)aeabi_memset.o: $(AEABI_MEMSET_DEP)

$(lpfx)aeabi_memset.obj: $(AEABI_MEMSET_DEP)

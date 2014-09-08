## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

if HAVE_THUMB1
if OPT_SIZE
STRCMP_SRC=strcmp.S
STRCMP_OBJ=$(lpfx)strcmp.o
STRLEN_SRC=strlen.c
STRLEN_OBJ=$(lpfx)strlen.o
else
STRCMP_SRC=
STRCMP_OBJ=
STRLEN_SRC=
STRLEN_OBJ=
endif
else
STRCMP_SRC=strcmp.S
STRCMP_OBJ=$(lpfx)strcmp.o
STRLEN_SRC=strlen.c
STRLEN_OBJ=$(lpfx)strlen.o
endif

if HAVE_ARMV7
MEMCHR_SRC=memchr.S
MEMCHR_OBJ=$(lpfx)memchr.o
else
MEMCHR_SRC=
MEMCHR_OBJ=
endif

if OPT_SIZE
MEMCPY_SRC=
MEMCPY_OBJ=
else
if HAVE_ARMV7A
MEMCPY_SRC=memcpy.S
MEMCPY_OBJ=$(lpfx)memcpy.o
else
if HAVE_ARMV7M
MEMCPY_SRC=memcpy.S
MEMCPY_OBJ=$(lpfx)memcpy.o
else
MEMCPY_SRC=
MEMCPY_OBJ=
endif !HAVE_ARMV7M
endif !HAVE_ARMV7A
endif !OPT_SIZE

lib_a_SOURCES = setjmp.S access.c $(STRCMP_SRC) strcpy.c \
	        $(MEMCPY_SRC) $(MEMCHR_SRC) $(STRLEN_SRC) \
		strlen-armv7.S aeabi_memcpy.c aeabi_memcpy-armv7a.S

lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_LIBADD = $(STRCMP_OBJ) $(STRLEN_OBJ) $(MEMCHR_OBJ) $(MEMCPY_OBJ)
lib_a_DEPENDENCIES = $(STRCMP_OBJ) $(STRLEN_OBJ) $(MEMCHR_OBJ) $(MEMCPY_OBJ)

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

MEMCPY_DEP=memcpy-armv7a.S memcpy-armv7m.S
STRCMP_DEP=strcmp-arm-tiny.S strcmp-armv4.S strcmp-armv4t.S strcmp-armv6.S \
	strcmp-armv7.S strcmp-armv7m.S

$(lpfx)memcpy.o: $(MEMCPY_DEP)

$(lpfx)memcpy.obj: $(MEMCPY_DEP)

$(lpfx)strcmp.o: $(STRCMP_DEP)

$(lpfx)strcmp.obj: $(STRCMP_DEP)

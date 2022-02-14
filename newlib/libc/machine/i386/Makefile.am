## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(AM_CPPFLAGS)

if MACH_ADD_SETJMP
ADDED_SOURCES = setjmp.S
else
ADDED_SOURCES =
endif

LIB_SOURCES = \
	memchr.S memcmp.S memcpy.S memset.S strchr.S \
	memmove.S strlen.S i386mach.h

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES) $(ADDED_SOURCES)
## These flag settings are basically redundant, but necessary to workaround the
## automake limitation "Objects 'created with both libtool and without'".
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

if MACH_ADD_SETJMP
mach_add_obj=setjmp.$(oext)
else
mach_add_obj=
endif

LIB_SOURCES = \
	memchr.S memcmp.S memcpy.S memset.S strchr.S \
	memmove.S strlen.S i386mach.h

libi386_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libi386.la
libi386_la_SOURCES = $(LIB_SOURCES)
EXTRA_libi386_la_SOURCES = setjmp.S
libi386_la_LIBADD = $(mach_add_obj)
libi386_la_DEPENDENCIES = $(mach_add_obj)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_LIBADD = $(mach_add_obj)
lib_a_DEPENDENCIES = $(mach_add_obj)
EXTRA_lib_a_SOURCES = setjmp.S
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

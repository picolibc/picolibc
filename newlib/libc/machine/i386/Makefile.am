## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

if MACH_ADD_SETJMP
mach_add_src=setjmp.S
else
mach_add_src=
endif

lib_a_SOURCES = \
	memchr.S memcmp.S memcpy.S memset.S strchr.S \
	$(mach_add_src)

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

# hack: putting $(newlib_basedir)/libm/common into INCLUDES forces automake
# to output a definition for newlib_basedir.
INCLUDES = -I $(newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = feclearexcept.c feraiseexcept.c feupdateenv.c	\
	fegetround.c fegetenv.c feholdexcept.c fetestexcept.c	\
	fegetexceptflag.c fegetexcept.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	feclearexcept.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c fenv.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

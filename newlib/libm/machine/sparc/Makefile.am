## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = -I $(abs_newlib_basedir)/libm/common $(NEWLIB_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	feclearexcept.c  fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c \
	fenv.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)

## Process this file with automake to generate Makefile.in

INCLUDES = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	feclearexcept.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

LIBADD_OBJS = \
	lib_a-fenv.$(OBJEXT)

# fenv.c cannot be compiled as mips16 since it uses the cfc1 instruction
lib_a-fenv.o: fenv.c
	$(COMPILE) -mno-mips16 -c -o $@ `test -f 'fenv.c' || echo '$(srcdir)/'`fenv.c

lib_a-fenv.obj: fenv.c
	$(COMPILE) -mno-mips16 -c -o $@ `if test -f 'fenv.c'; then $(CYGPATH_W) 'fenv.c'; else $(CYGPATH_W) '$(srcdir)/fenv.c'; fi`

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

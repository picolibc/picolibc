## Process this file with automake to generate Makefile.in

AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src =	feclearexcept.c fe_dfl_env.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c

lib_a_CFLAGS = -fbuiltin -fno-math-errno

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src)
lib_a_CFLAGS += $(AM_CFLAGS)

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/fenv.h

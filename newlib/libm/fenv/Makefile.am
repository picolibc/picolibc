## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src =	feclearexcept.c fe_dfl_env.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c

libcommon_la_LDFLAGS = -Xcompiler -nostdlib
lib_a_CFLAGS = -fbuiltin -fno-math-errno

if USE_LIBTOOL
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = $(src)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src)
lib_a_CFLAGS += $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES =	feclearexcept.def fegetenv.def \
	fegetexceptflag.def fegetround.def feholdexcept.def \
	feraiseexcept.def fesetenv.def fesetexceptflag.def fesetround.def \
	fetestexcept.def feupdateenv.def

CHAPTERS = fenv.tex

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/fenv.h

## Process this file with automake to generate Makefile.in

INCLUDES = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	feclearexcept.c fe_dfl_env.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c \
	s_fma.c e_sqrt.c sf_fma.c ef_sqrt.c \
	s_fmax.c s_fmin.c sf_fmax.c sf_fmin.c \
	s_fabs.c sf_fabs.c \
	s_fpclassify.c sf_fpclassify.c \
	s_finite.c sf_finite.c \
	s_isinf.c sf_isinf.c \
	s_isnan.c sf_isnan.c \
	s_copysign.c sf_copysign.c \
	s_lrint.c sf_lrint.c \
	s_lround.c sf_lround.c \
	s_llrint.c sf_llrint.c \
	s_llround.c sf_llround.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

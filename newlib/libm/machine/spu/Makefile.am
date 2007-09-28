## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I $(newlib_basedir)/../newlib/libm/common $(NEWLIB_CFLAGS) \
	$(CROSS_CFLAGS) $(TARGET_CFLAGS)

LIB_SOURCES = \
	feclearexcept.c fe_dfl_env.c fegetenv.c fegetexceptflag.c \
	fegetround.c feholdexcept.c feraiseexcept.c fesetenv.c \
	fesetexceptflag.c fesetround.c fetestexcept.c feupdateenv.c \
	llrint.c llrintf.c llround.c llroundf.c log2f.c s_cbrt.c s_ceil.c \
	s_copysign.c s_fabs.c sf_atan.c sf_cbrt.c sf_ceil.c sf_copysign.c \
	s_fdim.c sf_fabs.c sf_fdim.c sf_finite.c sf_floor.c sf_fma.c \
	sf_fmax.c sf_fmin.c sf_fpclassify.c sf_frexp.c sf_ilogb.c \
	sf_isinf.c sf_isinff.c sf_isnan.c sf_isnanf.c sf_ldexp.c s_floor.c \
	sf_lrint.c sf_lround.c s_fma.c s_fmax.c s_fmin.c sf_nan.c \
	sf_remquo.c s_frexp.c sf_round.c sf_scalbn.c sf_trunc.c s_ilogb.c \
	s_ldexp.c s_lrint.c s_lround.c s_nearbyint.c s_remquo.c s_rint.c \
	s_round.c s_scalbn.c s_trunc.c wf_exp2.c wf_exp.c wf_fmod.c \
	wf_log10.c wf_log.c w_fmod.c wf_remainder.c w_remainder.c \
	w_sqrt.c

noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(LIB_SOURCES)
lib_a_CFLAGS = $(AM_CFLAGS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
noinst_DATA =

include $(srcdir)/../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src =	s_acos.c s_frexp.c s_mathcnst.c \
	s_cos.c s_sinh.c \
	s_asin.c\
	s_asine.c s_cosh.c s_ispos.c s_numtest.c s_sqrt.c \
	s_exp.c s_ldexp.c s_pow.c s_tan.c \
	s_atan.c \
	s_atan2.c s_fabs.c s_log.c s_tanh.c \
	s_log10.c s_sin.c \
	s_floor.c s_sine.c \
	s_sincos.c \
	s_atangent.c s_logarithm.c \
	s_sineh.c \
	s_ceil.c \
	e_acosh.c e_atanh.c e_remainder.c \
	er_gamma.c er_lgamma.c \
	s_erf.c e_j0.c e_j1.c w_jn.c e_hypot.c \
	w_cabs.c w_drem.c s_asinh.c s_fmod.c \
	e_scalb.c s_signif.c \
	s_exp2.c s_tgamma.c

fsrc =	sf_ceil.c \
	sf_acos.c sf_frexp.c \
	sf_cos.c sf_sinh.c \
	sf_asine.c sf_cosh.c sf_ispos.c sf_numtest.c sf_sqrt.c \
	sf_asin.c \
	sf_exp.c sf_ldexp.c sf_pow.c sf_tan.c \
	sf_atan2.c sf_fabs.c sf_tanh.c \
	sf_atan.c sf_log10.c sf_sin.c\
	sf_floor.c sf_sine.c \
	sf_sincos.c \
	sf_atangent.c sf_logarithm.c sf_sineh.c \
	sf_log.c sf_sineh.c \
	ef_acosh.c ef_atanh.c ef_remainder.c \
	erf_gamma.c erf_lgamma.c \
	sf_erf.c ef_j0.c ef_j1.c wf_jn.c ef_hypot.c \
	wf_cabs.c wf_drem.c sf_asinh.c sf_fmod.c \
	ef_scalb.c sf_signif.c \
	sf_exp2.c sf_tgamma.c

libmathfp_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libmathfp.la
libmathfp_la_SOURCES = $(src) $(fsrc)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = e_acosh.def \
	e_atanh.def \
	e_hypot.def \
	e_remainder.def \
	er_lgamma.def \
	s_acos.def \
	s_asine.def \
	s_asinh.def \
	s_atan.def \
	s_atan2.def \
	s_atangent.def \
	s_cosh.def \
	s_erf.def \
	s_exp.def \
	s_fabs.def \
	s_floor.def \
	s_fmod.def \
	s_frexp.def \
	s_ldexp.def \
	s_log10.def \
	s_logarithm.def \
	s_pow.def \
	s_sine.def \
	s_sineh.def \
	s_sqrt.def \
	s_tan.def \
	s_tanh.def \
	w_jn.def

CHAPTERS = mathfp.tex

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/math.h $(srcdir)/../common/fdlibm.h

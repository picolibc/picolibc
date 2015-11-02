## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = -I$(srcdir)/../common $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src = 	k_standard.c k_rem_pio2.c \
	k_cos.c k_sin.c k_tan.c \
	e_acos.c e_acosh.c e_asin.c e_atan2.c \
	e_atanh.c e_cosh.c e_exp.c e_fmod.c \
	er_gamma.c e_hypot.c e_j0.c \
	e_j1.c e_jn.c er_lgamma.c \
	e_log.c e_log10.c e_pow.c e_rem_pio2.c e_remainder.c \
	e_scalb.c e_sinh.c e_sqrt.c \
	w_acos.c w_acosh.c w_asin.c w_atan2.c \
	w_atanh.c w_cosh.c w_exp.c w_fmod.c \
	w_gamma.c wr_gamma.c w_hypot.c w_j0.c \
	w_j1.c w_jn.c w_lgamma.c wr_lgamma.c \
	w_log.c w_log10.c w_pow.c w_remainder.c \
	w_scalb.c w_sinh.c w_sqrt.c \
	w_sincos.c \
	w_drem.c \
	s_asinh.c s_atan.c s_ceil.c \
	s_cos.c s_erf.c s_fabs.c s_floor.c \
	s_frexp.c s_ldexp.c \
	s_signif.c s_sin.c \
	s_tan.c s_tanh.c \
	w_exp2.c w_tgamma.c

fsrc =	kf_rem_pio2.c \
	kf_cos.c kf_sin.c kf_tan.c \
	ef_acos.c ef_acosh.c ef_asin.c ef_atan2.c \
	ef_atanh.c ef_cosh.c ef_exp.c ef_fmod.c \
	erf_gamma.c ef_hypot.c ef_j0.c \
	ef_j1.c ef_jn.c erf_lgamma.c \
	ef_log.c ef_log10.c ef_pow.c ef_rem_pio2.c ef_remainder.c \
	ef_scalb.c ef_sinh.c ef_sqrt.c \
	wf_acos.c wf_acosh.c wf_asin.c wf_atan2.c \
	wf_atanh.c wf_cosh.c wf_exp.c wf_fmod.c \
	wf_gamma.c wrf_gamma.c wf_hypot.c wf_j0.c \
	wf_j1.c wf_jn.c wf_lgamma.c wrf_lgamma.c \
	wf_log.c wf_log10.c wf_pow.c wf_remainder.c \
	wf_scalb.c wf_sinh.c wf_sqrt.c \
	wf_sincos.c \
	wf_drem.c \
	sf_asinh.c sf_atan.c sf_ceil.c \
	sf_cos.c sf_erf.c sf_fabs.c sf_floor.c \
	sf_frexp.c sf_ldexp.c \
	sf_signif.c sf_sin.c \
	sf_tan.c sf_tanh.c \
	wf_exp2.c wf_tgamma.c

lsrc = el_hypot.c

libmath_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libmath.la
libmath_la_SOURCES = $(src) $(fsrc) $(lsrc)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc) $(lsrc)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES = w_acos.def w_acosh.def w_asin.def s_asinh.def \
	s_atan.def w_atan2.def w_atanh.def w_j0.def \
	w_cosh.def s_erf.def w_exp.def w_exp2.def \
	s_fabs.def s_floor.def w_fmod.def s_frexp.def \
	w_gamma.def w_hypot.def s_ldexp.def w_log.def \
	w_log10.def \
	w_pow.def w_remainder.def s_sin.def w_sinh.def \
	w_sqrt.def s_tan.def s_tanh.def

CHAPTERS = math.tex

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/math.h $(srcdir)/../common/fdlibm.h

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

src = 	s_finite.c s_copysign.c s_modf.c s_scalbn.c \
	s_cbrt.c s_exp10.c s_expm1.c s_ilogb.c \
	s_infinity.c s_isinf.c s_isinfd.c s_isnan.c s_isnand.c \
	s_log1p.c s_nan.c s_nextafter.c s_pow10.c \
	s_rint.c s_logb.c s_log2.c s_matherr.c s_lib_ver.c \
	s_fdim.c s_fma.c s_fmax.c s_fmin.c s_fpclassify.c \
	s_lrint.c s_llrint.c \
	s_lround.c s_llround.c s_nearbyint.c s_remquo.c s_round.c s_scalbln.c \
	s_signbit.c s_trunc.c \
	exp.c exp2.c exp_data.c math_err.c log.c log_data.c log2.c log2_data.c \
	pow.c pow_log_data.c

fsrc =	sf_finite.c sf_copysign.c sf_modf.c sf_scalbn.c \
	sf_cbrt.c sf_exp10.c sf_expm1.c sf_ilogb.c \
	sf_infinity.c sf_isinf.c sf_isinff.c sf_isnan.c sf_isnanf.c \
	sf_log1p.c sf_nan.c sf_nextafter.c sf_pow10.c \
	sf_rint.c sf_logb.c \
	sf_fdim.c sf_fma.c sf_fmax.c sf_fmin.c sf_fpclassify.c \
	sf_lrint.c sf_llrint.c \
	sf_lround.c sf_llround.c sf_nearbyint.c sf_remquo.c sf_round.c \
	sf_scalbln.c sf_trunc.c \
	sf_exp.c sf_exp2.c sf_exp2_data.c sf_log.c sf_log_data.c \
	sf_log2.c sf_log2_data.c sf_pow_log2_data.c sf_pow.c \
	sinf.c cosf.c sincosf.c sincosf_data.c math_errf.c

lsrc =	atanl.c cosl.c sinl.c tanl.c tanhl.c frexpl.c modfl.c ceill.c fabsl.c \
	floorl.c log1pl.c expm1l.c acosl.c asinl.c atan2l.c coshl.c sinhl.c \
	expl.c ldexpl.c logl.c log10l.c powl.c sqrtl.c fmodl.c hypotl.c \
	copysignl.c nanl.c ilogbl.c asinhl.c cbrtl.c nextafterl.c rintl.c \
	scalbnl.c exp2l.c scalblnl.c tgammal.c nearbyintl.c lrintl.c llrintl.c \
	roundl.c lroundl.c llroundl.c truncl.c remquol.c fdiml.c fmaxl.c fminl.c \
	fmal.c acoshl.c atanhl.c remainderl.c lgammal.c erfl.c erfcl.c \
	logbl.c nexttowardf.c nexttoward.c nexttowardl.c log2l.c \
	sl_finite.c

libcommon_la_LDFLAGS = -Xcompiler -nostdlib
lib_a_CFLAGS = -fbuiltin -fno-math-errno

if USE_LIBTOOL
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = $(src) $(fsrc)
if HAVE_LONG_DOUBLE
libcommon_la_SOURCES += $(lsrc)
endif # HAVE_LONG_DOUBLE
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a
lib_a_SOURCES = $(src) $(fsrc)
if HAVE_LONG_DOUBLE
lib_a_SOURCES += $(lsrc)
endif # HAVE_LONG_DOUBLE
lib_a_CFLAGS += $(AM_CFLAGS)
noinst_DATA =
endif # USE_LIBTOOL

include $(srcdir)/../../Makefile.shared

CHEWOUT_FILES =	s_cbrt.def s_copysign.def s_exp10.def s_expm1.def s_ilogb.def \
	s_infinity.def s_isnan.def s_log1p.def s_matherr.def s_modf.def \
	s_nan.def s_nextafter.def s_pow10.def s_scalbn.def \
	s_fdim.def s_fma.def s_fmax.def s_fmin.def \
	s_logb.def s_log2.def s_lrint.def s_lround.def s_nearbyint.def \
	s_remquo.def s_rint.def s_round.def s_signbit.def s_trunc.def \
	isgreater.def

CHAPTERS =

# A partial dependency list.

$(lib_a_OBJECTS): $(srcdir)/../../libc/include/math.h fdlibm.h

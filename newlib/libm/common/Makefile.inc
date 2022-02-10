%C%_src = \
	%D%/s_finite.c \
	%D%/s_copysign.c \
	%D%/s_modf.c \
	%D%/s_scalbn.c \
	%D%/s_cbrt.c \
	%D%/s_exp10.c \
	%D%/s_expm1.c \
	%D%/s_ilogb.c \
	%D%/s_infinity.c \
	%D%/s_isinf.c \
	%D%/s_isinfd.c \
	%D%/s_isnan.c \
	%D%/s_isnand.c \
	%D%/s_log1p.c \
	%D%/s_nan.c \
	%D%/s_nextafter.c \
	%D%/s_pow10.c \
	%D%/s_rint.c \
	%D%/s_logb.c \
	%D%/s_log2.c \
	%D%/s_fdim.c \
	%D%/s_fma.c \
	%D%/s_fmax.c \
	%D%/s_fmin.c \
	%D%/s_fpclassify.c \
	%D%/s_lrint.c \
	%D%/s_llrint.c \
	%D%/s_lround.c \
	%D%/s_llround.c \
	%D%/s_nearbyint.c \
	%D%/s_remquo.c \
	%D%/s_round.c \
	%D%/s_scalbln.c \
	%D%/s_signbit.c \
	%D%/s_trunc.c \
	%D%/exp.c \
	%D%/exp2.c \
	%D%/exp_data.c \
	%D%/math_err.c \
	%D%/log.c \
	%D%/log_data.c \
	%D%/log2.c \
	%D%/log2_data.c \
	%D%/pow.c \
	%D%/pow_log_data.c

%C%_fsrc = \
	%D%/sf_finite.c %D%/sf_copysign.c %D%/sf_modf.c %D%/sf_scalbn.c \
	%D%/sf_cbrt.c %D%/sf_exp10.c %D%/sf_expm1.c %D%/sf_ilogb.c \
	%D%/sf_infinity.c %D%/sf_isinf.c %D%/sf_isinff.c %D%/sf_isnan.c %D%/sf_isnanf.c \
	%D%/sf_log1p.c %D%/sf_nan.c %D%/sf_nextafter.c %D%/sf_pow10.c \
	%D%/sf_rint.c %D%/sf_logb.c \
	%D%/sf_fdim.c %D%/sf_fma.c %D%/sf_fmax.c %D%/sf_fmin.c %D%/sf_fpclassify.c \
	%D%/sf_lrint.c %D%/sf_llrint.c \
	%D%/sf_lround.c %D%/sf_llround.c %D%/sf_nearbyint.c %D%/sf_remquo.c %D%/sf_round.c \
	%D%/sf_scalbln.c %D%/sf_trunc.c \
	%D%/sf_exp.c %D%/sf_exp2.c %D%/sf_exp2_data.c %D%/sf_log.c %D%/sf_log_data.c \
	%D%/sf_log2.c %D%/sf_log2_data.c %D%/sf_pow_log2_data.c %D%/sf_pow.c \
	%D%/sinf.c %D%/cosf.c %D%/sincosf.c %D%/sincosf_data.c %D%/math_errf.c

%C%_lsrc = \
	%D%/atanl.c %D%/cosl.c %D%/sinl.c %D%/tanl.c %D%/tanhl.c %D%/frexpl.c %D%/modfl.c %D%/ceill.c %D%/fabsl.c \
	%D%/floorl.c %D%/log1pl.c %D%/expm1l.c %D%/acosl.c %D%/asinl.c %D%/atan2l.c %D%/coshl.c %D%/sinhl.c \
	%D%/expl.c %D%/ldexpl.c %D%/logl.c %D%/log10l.c %D%/powl.c %D%/sqrtl.c %D%/fmodl.c %D%/hypotl.c \
	%D%/copysignl.c %D%/nanl.c %D%/ilogbl.c %D%/asinhl.c %D%/cbrtl.c %D%/nextafterl.c %D%/rintl.c \
	%D%/scalbnl.c %D%/exp2l.c %D%/scalblnl.c %D%/tgammal.c %D%/nearbyintl.c %D%/lrintl.c %D%/llrintl.c \
	%D%/roundl.c %D%/lroundl.c %D%/llroundl.c %D%/truncl.c %D%/remquol.c %D%/fdiml.c %D%/fmaxl.c %D%/fminl.c \
	%D%/fmal.c %D%/acoshl.c %D%/atanhl.c %D%/remainderl.c %D%/lgammal.c %D%/erfl.c %D%/erfcl.c \
	%D%/logbl.c %D%/nexttowardf.c %D%/nexttoward.c %D%/nexttowardl.c %D%/log2l.c \
	%D%/sl_finite.c

libm_a_CFLAGS_%C% = -fbuiltin -fno-math-errno

libm_a_SOURCES += $(%C%_src) $(%C%_fsrc)
if HAVE_LONG_DOUBLE
libm_a_SOURCES += $(%C%_lsrc)
endif # HAVE_LONG_DOUBLE

LIBM_CHEWOUT_FILES += \
	%D%/s_cbrt.def %D%/s_copysign.def %D%/s_exp10.def %D%/s_expm1.def %D%/s_ilogb.def \
	%D%/s_infinity.def %D%/s_isnan.def %D%/s_log1p.def %D%/s_modf.def \
	%D%/s_nan.def %D%/s_nextafter.def %D%/s_pow10.def %D%/s_scalbn.def \
	%D%/s_fdim.def %D%/s_fma.def %D%/s_fmax.def %D%/s_fmin.def \
	%D%/s_logb.def %D%/s_log2.def %D%/s_lrint.def %D%/s_lround.def %D%/s_nearbyint.def \
	%D%/s_remquo.def %D%/s_rint.def %D%/s_round.def %D%/s_signbit.def %D%/s_trunc.def \
	%D%/isgreater.def

LIBM_CHAPTERS +=

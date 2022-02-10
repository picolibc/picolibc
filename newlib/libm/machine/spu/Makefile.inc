%C%_src = \
	%D%/feclearexcept.c %D%/fe_dfl_env.c %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c \
	%D%/llrint.c %D%/llrintf.c %D%/llround.c %D%/llroundf.c %D%/log2.c %D%/log2f.c %D%/s_asinh.c \
	%D%/s_atan.c %D%/s_cbrt.c %D%/s_ceil.c %D%/s_copysign.c %D%/s_cos.c %D%/s_erf.c %D%/s_expm1.c \
	%D%/s_fabs.c %D%/sf_asinh.c %D%/sf_atan.c %D%/sf_cbrt.c %D%/sf_ceil.c %D%/sf_copysign.c \
	%D%/sf_cos.c %D%/s_fdim.c %D%/sf_erf.c %D%/sf_expm1.c %D%/sf_fabs.c %D%/sf_fdim.c \
	%D%/sf_finite.c %D%/sf_floor.c %D%/sf_fma.c %D%/sf_fmax.c %D%/sf_fmin.c \
	%D%/sf_fpclassify.c %D%/sf_frexp.c %D%/sf_ilogb.c %D%/sf_isinf.c %D%/sf_isinff.c \
	%D%/sf_isnan.c %D%/sf_isnanf.c %D%/sf_ldexp.c %D%/sf_log1p.c %D%/sf_logb.c %D%/s_floor.c \
	%D%/sf_lrint.c %D%/sf_lround.c %D%/s_fma.c %D%/s_fmax.c %D%/s_fmin.c %D%/sf_nan.c \
	%D%/sf_nearbyint.c %D%/sf_nextafter.c %D%/sf_remquo.c %D%/s_frexp.c %D%/sf_rint.c \
	%D%/sf_round.c %D%/sf_scalbn.c %D%/sf_sin.c %D%/sf_tan.c %D%/sf_tanh.c %D%/sf_trunc.c \
	%D%/s_ilogb.c %D%/s_isnan.c %D%/s_ldexp.c %D%/s_log1p.c %D%/s_lrint.c %D%/s_lround.c \
	%D%/s_nearbyint.c %D%/s_nextafter.c %D%/s_remquo.c %D%/s_rint.c %D%/s_round.c \
	%D%/s_scalbn.c %D%/s_sin.c %D%/s_tan.c %D%/s_tanh.c %D%/s_trunc.c %D%/w_acos.c %D%/w_acosh.c \
	%D%/w_asin.c %D%/w_atan2.c %D%/w_atanh.c %D%/w_cosh.c %D%/w_exp2.c %D%/w_exp.c %D%/wf_acos.c \
	%D%/wf_acosh.c %D%/wf_asin.c %D%/wf_atan2.c %D%/wf_atanh.c %D%/wf_cosh.c %D%/wf_exp2.c \
	%D%/wf_exp.c %D%/wf_fmod.c %D%/wf_hypot.c %D%/wf_lgamma.c %D%/wf_log10.c %D%/wf_log.c \
	%D%/w_fmod.c %D%/wf_pow.c %D%/wf_remainder.c %D%/wf_sincos.c %D%/wf_sinh.c %D%/wf_sqrt.c \
	%D%/wf_tgamma.c %D%/w_hypot.c %D%/w_lgamma.c %D%/w_log10.c %D%/w_log.c %D%/w_pow.c \
	%D%/w_remainder.c %D%/w_sincos.c %D%/w_sinh.c %D%/w_sqrt.c %D%/w_tgamma.c

libm_a_SOURCES += $(%C%_src)

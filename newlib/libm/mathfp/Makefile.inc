%C%_src = \
	%D%/s_acos.c %D%/s_frexp.c %D%/s_mathcnst.c \
	%D%/s_cos.c %D%/s_sinh.c \
	%D%/s_asin.c\
	%D%/s_asine.c %D%/s_cosh.c %D%/s_ispos.c %D%/s_numtest.c %D%/s_sqrt.c \
	%D%/s_exp.c %D%/s_ldexp.c %D%/s_pow.c %D%/s_tan.c \
	%D%/s_atan.c \
	%D%/s_atan2.c %D%/s_fabs.c %D%/s_log.c %D%/s_tanh.c \
	%D%/s_log10.c %D%/s_sin.c \
	%D%/s_floor.c %D%/s_sine.c \
	%D%/s_sincos.c \
	%D%/s_atangent.c %D%/s_logarithm.c \
	%D%/s_sineh.c \
	%D%/s_ceil.c \
	%D%/e_acosh.c %D%/e_atanh.c %D%/e_remainder.c \
	%D%/er_gamma.c %D%/er_lgamma.c \
	%D%/s_erf.c %D%/e_j0.c %D%/e_j1.c %D%/w_jn.c %D%/e_hypot.c \
	%D%/w_cabs.c %D%/w_drem.c %D%/s_asinh.c %D%/s_fmod.c \
	%D%/e_scalb.c %D%/s_signif.c \
	%D%/s_exp2.c %D%/s_tgamma.c

%C%_fsrc = \
	%D%/sf_ceil.c \
	%D%/sf_acos.c %D%/sf_frexp.c \
	%D%/sf_cos.c %D%/sf_sinh.c \
	%D%/sf_asine.c %D%/sf_cosh.c %D%/sf_ispos.c %D%/sf_numtest.c %D%/sf_sqrt.c \
	%D%/sf_asin.c \
	%D%/sf_exp.c %D%/sf_ldexp.c %D%/sf_pow.c %D%/sf_tan.c \
	%D%/sf_atan2.c %D%/sf_fabs.c %D%/sf_tanh.c \
	%D%/sf_atan.c %D%/sf_log10.c %D%/sf_sin.c\
	%D%/sf_floor.c %D%/sf_sine.c \
	%D%/sf_sincos.c \
	%D%/sf_atangent.c %D%/sf_logarithm.c %D%/sf_sineh.c \
	%D%/sf_log.c %D%/sf_sineh.c \
	%D%/ef_acosh.c %D%/ef_atanh.c %D%/ef_remainder.c \
	%D%/erf_gamma.c %D%/erf_lgamma.c \
	%D%/sf_erf.c %D%/ef_j0.c %D%/ef_j1.c %D%/wf_jn.c %D%/ef_hypot.c \
	%D%/wf_cabs.c %D%/wf_drem.c %D%/sf_asinh.c %D%/sf_fmod.c \
	%D%/ef_scalb.c %D%/sf_signif.c \
	%D%/sf_exp2.c %D%/sf_tgamma.c

libm_a_SOURCES += $(%C%_src) $(%C%_fsrc)

LIBM_CHEWOUT_FILES += \
	%D%/e_acosh.def \
	%D%/e_atanh.def \
	%D%/e_hypot.def \
	%D%/e_remainder.def \
	%D%/er_lgamma.def \
	%D%/s_acos.def \
	%D%/s_asine.def \
	%D%/s_asinh.def \
	%D%/s_atan.def \
	%D%/s_atan2.def \
	%D%/s_atangent.def \
	%D%/s_cosh.def \
	%D%/s_erf.def \
	%D%/s_exp.def \
	%D%/s_fabs.def \
	%D%/s_floor.def \
	%D%/s_fmod.def \
	%D%/s_frexp.def \
	%D%/s_ldexp.def \
	%D%/s_log10.def \
	%D%/s_logarithm.def \
	%D%/s_pow.def \
	%D%/s_sine.def \
	%D%/s_sineh.def \
	%D%/s_sqrt.def \
	%D%/s_tan.def \
	%D%/s_tanh.def \
	%D%/w_jn.def

LIBM_CHAPTERS += %D%/mathfp.tex

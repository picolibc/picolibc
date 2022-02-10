%C%_src = \
	%D%/k_standard.c %D%/k_rem_pio2.c \
	%D%/k_cos.c %D%/k_sin.c %D%/k_tan.c \
	%D%/e_acos.c %D%/e_acosh.c %D%/e_asin.c %D%/e_atan2.c \
	%D%/e_atanh.c %D%/e_cosh.c %D%/e_exp.c %D%/e_fmod.c \
	%D%/e_tgamma.c %D%/e_hypot.c %D%/e_j0.c \
	%D%/e_j1.c %D%/e_jn.c %D%/er_lgamma.c \
	%D%/e_log.c %D%/e_log10.c %D%/e_pow.c %D%/e_rem_pio2.c %D%/e_remainder.c \
	%D%/e_scalb.c %D%/e_sinh.c %D%/e_sqrt.c \
	%D%/w_acos.c %D%/w_acosh.c %D%/w_asin.c %D%/w_atan2.c \
	%D%/w_atanh.c %D%/w_cosh.c %D%/w_exp.c %D%/w_fmod.c \
	%D%/w_gamma.c %D%/wr_gamma.c %D%/w_hypot.c %D%/w_j0.c \
	%D%/w_j1.c %D%/w_jn.c %D%/w_lgamma.c %D%/wr_lgamma.c \
	%D%/w_log.c %D%/w_log10.c %D%/w_pow.c %D%/w_remainder.c \
	%D%/w_scalb.c %D%/w_sinh.c %D%/w_sqrt.c \
	%D%/w_sincos.c \
	%D%/w_drem.c \
	%D%/s_asinh.c %D%/s_atan.c %D%/s_ceil.c \
	%D%/s_cos.c %D%/s_erf.c %D%/s_fabs.c %D%/s_floor.c \
	%D%/s_frexp.c %D%/s_ldexp.c \
	%D%/s_signif.c %D%/s_sin.c \
	%D%/s_tan.c %D%/s_tanh.c \
	%D%/w_exp2.c %D%/w_tgamma.c

%C%_fsrc = \
	%D%/kf_rem_pio2.c \
	%D%/kf_cos.c %D%/kf_sin.c %D%/kf_tan.c \
	%D%/ef_acos.c %D%/ef_acosh.c %D%/ef_asin.c %D%/ef_atan2.c \
	%D%/ef_atanh.c %D%/ef_cosh.c %D%/ef_exp.c %D%/ef_fmod.c \
	%D%/ef_tgamma.c %D%/ef_hypot.c %D%/ef_j0.c \
	%D%/ef_j1.c %D%/ef_jn.c %D%/erf_lgamma.c \
	%D%/ef_log.c %D%/ef_log10.c %D%/ef_pow.c %D%/ef_rem_pio2.c %D%/ef_remainder.c \
	%D%/ef_scalb.c %D%/ef_sinh.c %D%/ef_sqrt.c \
	%D%/wf_acos.c %D%/wf_acosh.c %D%/wf_asin.c %D%/wf_atan2.c \
	%D%/wf_atanh.c %D%/wf_cosh.c %D%/wf_exp.c %D%/wf_fmod.c \
	%D%/wf_gamma.c %D%/wrf_gamma.c %D%/wf_hypot.c %D%/wf_j0.c \
	%D%/wf_j1.c %D%/wf_jn.c %D%/wf_lgamma.c %D%/wrf_lgamma.c \
	%D%/wf_log.c %D%/wf_log10.c %D%/wf_pow.c %D%/wf_remainder.c \
	%D%/wf_scalb.c %D%/wf_sinh.c %D%/wf_sqrt.c \
	%D%/wf_sincos.c \
	%D%/wf_drem.c \
	%D%/sf_asinh.c %D%/sf_atan.c %D%/sf_ceil.c \
	%D%/sf_cos.c %D%/sf_erf.c %D%/sf_fabs.c %D%/sf_floor.c \
	%D%/sf_frexp.c %D%/sf_ldexp.c \
	%D%/sf_signif.c %D%/sf_sin.c \
	%D%/sf_tan.c %D%/sf_tanh.c \
	%D%/wf_exp2.c %D%/wf_tgamma.c \
	%D%/wf_log2.c

%C%_lsrc = %D%/el_hypot.c

libm_a_SOURCES += $(%C%_src) $(%C%_fsrc) $(%C%_lsrc)

LIBM_CHEWOUT_FILES += \
	%D%/w_acos.def %D%/w_acosh.def %D%/w_asin.def %D%/s_asinh.def \
	%D%/s_atan.def %D%/w_atan2.def %D%/w_atanh.def %D%/w_j0.def \
	%D%/w_cosh.def %D%/s_erf.def %D%/w_exp.def %D%/w_exp2.def \
	%D%/s_fabs.def %D%/s_floor.def %D%/w_fmod.def %D%/s_frexp.def \
	%D%/w_gamma.def %D%/w_hypot.def %D%/s_ldexp.def %D%/w_log.def \
	%D%/w_log10.def \
	%D%/w_pow.def %D%/w_remainder.def %D%/s_sin.def %D%/w_sinh.def \
	%D%/w_sqrt.def %D%/s_tan.def %D%/s_tanh.def

LIBM_CHAPTERS += %D%/math.tex

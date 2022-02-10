%C%_src = \
	%D%/feclearexcept.c %D%/fe_dfl_env.c %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c \
	%D%/s_fma.c %D%/e_sqrt.c %D%/sf_fma.c %D%/ef_sqrt.c \
	%D%/s_fmax.c %D%/s_fmin.c %D%/sf_fmax.c %D%/sf_fmin.c \
	%D%/s_fabs.c %D%/sf_fabs.c \
	%D%/s_fpclassify.c %D%/sf_fpclassify.c \
	%D%/s_finite.c %D%/sf_finite.c \
	%D%/s_isinf.c %D%/sf_isinf.c \
	%D%/s_isnan.c %D%/sf_isnan.c \
	%D%/s_copysign.c %D%/sf_copysign.c \
	%D%/s_lrint.c %D%/sf_lrint.c \
	%D%/s_lround.c %D%/sf_lround.c \
	%D%/s_llrint.c %D%/sf_llrint.c \
	%D%/s_llround.c %D%/sf_llround.c

libm_a_SOURCES += $(%C%_src)

%C%_src = \
	%D%/e_sqrt.c \
	%D%/ef_sqrt.c \
	%D%/s_ceil.c \
	%D%/s_fabs.c \
	%D%/s_floor.c \
	%D%/s_fma.c \
	%D%/s_fmax.c \
	%D%/s_fmin.c \
	%D%/s_llrint.c \
	%D%/s_llround.c \
	%D%/s_lrint.c \
	%D%/s_lround.c \
	%D%/s_nearbyint.c \
	%D%/s_rint.c \
	%D%/s_round.c \
	%D%/s_trunc.c \
	%D%/sf_ceil.c \
	%D%/sf_fabs.c \
	%D%/sf_floor.c \
	%D%/sf_fma.c \
	%D%/sf_fmax.c \
	%D%/sf_fmin.c \
	%D%/sf_llrint.c \
	%D%/sf_llround.c \
	%D%/sf_lrint.c \
	%D%/sf_lround.c \
	%D%/sf_nearbyint.c \
	%D%/sf_rint.c \
	%D%/sf_round.c \
	%D%/sf_trunc.c \
	%D%/fenv.c \
	%D%/feclearexcept.c \
	%D%/fegetenv.c \
	%D%/fegetexceptflag.c \
	%D%/fegetround.c \
	%D%/feholdexcept.c \
	%D%/feraiseexcept.c \
	%D%/fesetenv.c \
	%D%/fesetexceptflag.c \
	%D%/fesetround.c \
	%D%/fetestexcept.c \
	%D%/feupdateenv.c

libm_a_SOURCES += $(%C%_src)

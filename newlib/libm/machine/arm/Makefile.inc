%C%_src = \
	%D%/e_sqrt.c \
	%D%/ef_sqrt.c \
	%D%/s_ceil.c \
	%D%/s_floor.c \
	%D%/s_fma_arm.c \
	%D%/s_nearbyint.c \
	%D%/s_rint.c \
	%D%/s_round.c \
	%D%/s_trunc.c \
	%D%/sf_ceil.c \
	%D%/sf_floor.c \
	%D%/sf_fma_arm.c \
	%D%/sf_nearbyint.c \
	%D%/sf_rint.c \
	%D%/sf_round.c \
	%D%/sf_trunc.c \
	%D%/feclearexcept.c \
	%D%/fe_dfl_env.c\
	%D%/fegetenv.c \
	%D%/fegetexceptflag.c \
	%D%/fegetround.c \
	%D%/feholdexcept.c \
	%D%/fegetexcept.c \
	%D%/feraiseexcept.c \
	%D%/fesetenv.c \
	%D%/fesetexceptflag.c \
	%D%/fesetround.c \
	%D%/fetestexcept.c \
	%D%/feupdateenv.c \
	%D%/feenableexcept.c \
	%D%/fedisableexcept.c

libm_a_SOURCES += $(%C%_src)

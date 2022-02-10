%C%_src = \
	%D%/feclearexcept.c %D%/fe_dfl_env.c %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c

libm_a_SOURCES += $(%C%_src)

LIBM_CHEWOUT_FILES += \
	%D%/feclearexcept.def %D%/fegetenv.def \
	%D%/fegetexceptflag.def %D%/fegetround.def %D%/feholdexcept.def \
	%D%/feraiseexcept.def %D%/fesetenv.def %D%/fesetexceptflag.def %D%/fesetround.def \
	%D%/fetestexcept.def %D%/feupdateenv.def

LIBM_CHAPTERS += %D%/fenv.tex

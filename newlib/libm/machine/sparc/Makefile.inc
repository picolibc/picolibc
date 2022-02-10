%C%_src = \
	%D%/feclearexcept.c  %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c \
	%D%/fenv.c

libm_a_SOURCES += $(%C%_src)

%C%_src = \
	%D%/feclearexcept.c %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c \
	%D%/fenv.c

libm_a_SOURCES += $(%C%_src)

# fenv.c cannot be compiled as mips16 since it uses the cfc1 instruction.
libm_a_CFLAGS_%C%_fenv.c = -mno-mips16

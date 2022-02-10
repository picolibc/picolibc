%C%_src = \
	%D%/f_atan2.S %D%/f_atan2f.S %D%/f_exp.c %D%/f_expf.c \
	%D%/f_frexp.S %D%/f_frexpf.S %D%/f_llrint.c %D%/f_llrintf.c %D%/f_llrintl.c \
	%D%/f_log.S %D%/f_logf.S %D%/f_log10.S %D%/f_log10f.S \
	%D%/f_ldexp.S %D%/f_ldexpf.S %D%/f_lrint.c %D%/f_lrintf.c %D%/f_lrintl.c \
	%D%/f_pow.c %D%/f_powf.c %D%/f_rint.c %D%/f_rintf.c %D%/f_rintl.c \
	%D%/f_tan.S %D%/f_tanf.S %D%/f_math.h %D%/i386mach.h \
	%D%/fenv.c %D%/feclearexcept.c %D%/fegetenv.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fesetenv.c \
	%D%/fesetexceptflag.c %D%/fesetround.c %D%/fetestexcept.c %D%/feupdateenv.c

libm_a_SOURCES += $(%C%_src)

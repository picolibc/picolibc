%C%_src = \
	%D%/feclearexcept.c %D%/fegetenv.c %D%/fegetexcept.c %D%/fegetexceptflag.c \
	%D%/fegetround.c %D%/feholdexcept.c %D%/feraiseexcept.c %D%/fetestexcept.c \
	%D%/feupdateenv.c

if XTENSA_XCHAL_HAVE_FP_SQRT
%C%_src += \
	%D%/ef_sqrt.c
endif

libm_a_CFLAGS_%C% = -D_LIBM
libm_a_SOURCES += $(%C%_src)

%C%_lsrc = \
	%D%/e_acoshl.c \
	%D%/e_acosl.c \
	%D%/e_asinl.c \
	%D%/e_atan2l.c \
	%D%/e_atanhl.c \
	%D%/e_coshl.c \
	%D%/e_fmodl.c \
	%D%/e_lgammal.c \
	%D%/e_remainderl.c \
	%D%/e_sinhl.c \
	%D%/s_asinhl.c \
	%D%/s_atanl.c \
	%D%/s_cbrtl.c \
	%D%/s_ceill.c \
	%D%/s_copysignl.c \
	%D%/s_cosl.c \
	%D%/s_fabsl.c \
	%D%/fdiml.c \
	%D%/s_floorl.c \
	%D%/s_fmal.c \
	%D%/s_fmaxl.c \
	%D%/s_fminl.c \
	%D%/s_frexpl.c \
	%D%/s_ilogbl.c \
	%D%/s_llrintl.c \
	%D%/s_llroundl.c \
	%D%/s_logbl.c \
	%D%/s_lrint.c \
	%D%/s_lrintl.c \
	%D%/s_lround.c \
	%D%/s_lroundl.c \
	%D%/s_modfl.c \
	%D%/s_nearbyintl.c \
	%D%/s_nextafterl.c \
	%D%/s_nexttoward.c \
	%D%/s_nexttowardf.c \
	%D%/s_remquol.c \
	%D%/s_rintl.c \
	%D%/s_roundl.c \
	%D%/scalblnl.c \
	%D%/s_scalbnl.c \
	%D%/s_sinl.c \
	%D%/s_tanhl.c \
	%D%/s_tanl.c \
	%D%/s_truncl.c

libm_a_CFLAGS_%C% = -fbuiltin -fno-math-errno

if HAVE_LONG_DOUBLE
libm_a_SOURCES += $(%C%_lsrc)
endif # HAVE_LONG_DOUBLE

LIBM_CHEWOUT_FILES +=

LIBM_CHAPTERS +=

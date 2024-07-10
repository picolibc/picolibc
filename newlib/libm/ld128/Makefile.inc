%C%_lsrc = \
	%D%/e_powl.c %D%/s_erfl.c %D%/s_exp2l.c %D%/s_expl.c \
	%D%/s_logl.c %D%/b_tgammal.c %D%/invtrig.c \
	%D%/e_lgammal_r.c %D%/k_cosl.c %D%/k_sinl.c \
	%D%/k_tanl.c %D%/s_sinpil.c %D%/s_cospil.c

libm_a_CFLAGS_%C% = -fbuiltin -fno-math-errno

if HAVE_LONG_DOUBLE
libm_a_SOURCES += $(%C%_lsrc)
endif # HAVE_LONG_DOUBLE

LIBM_CHEWOUT_FILES +=

LIBM_CHAPTERS +=

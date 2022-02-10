%C%_src = \
	%D%/fpclassify.c %D%/fpclassifyf.c %D%/isfinite.c %D%/isfinitef.c %D%/isinf.c \
	%D%/isinff.c %D%/isnan.c %D%/isnanf.c %D%/isnormal.c %D%/isnormalf.c

libm_a_SOURCES += $(%C%_src)

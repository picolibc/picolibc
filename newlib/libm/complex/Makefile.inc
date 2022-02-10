%C%_src = \
	%D%/cabs.c %D%/cacos.c %D%/cacosh.c %D%/carg.c %D%/casin.c %D%/casinh.c \
	%D%/catan.c %D%/catanh.c %D%/ccos.c %D%/ccosh.c %D%/cephes_subr.c \
	%D%/cexp.c %D%/cimag.c %D%/clog.c %D%/clog10.c %D%/conj.c \
	%D%/cpow.c %D%/cproj.c %D%/creal.c \
	%D%/csin.c %D%/csinh.c %D%/csqrt.c %D%/ctan.c %D%/ctanh.c

%C%_lsrc = \
	%D%/cabsl.c %D%/creall.c %D%/cimagl.c %D%/ccoshl.c %D%/cacoshl.c \
	%D%/clogl.c %D%/csqrtl.c %D%/cargl.c %D%/cprojl.c %D%/cexpl.c \
	%D%/cephes_subrl.c %D%/cacosl.c %D%/ccosl.c %D%/casinl.c \
	%D%/catanhl.c %D%/conjl.c %D%/cpowl.c %D%/ctanhl.c %D%/ctanl.c \
	%D%/casinhl.c %D%/csinhl.c %D%/csinl.c %D%/catanl.c

%C%_fsrc = \
	%D%/cabsf.c %D%/casinf.c %D%/ccosf.c %D%/cimagf.c %D%/cprojf.c \
	%D%/csqrtf.c %D%/cacosf.c %D%/casinhf.c %D%/ccoshf.c %D%/clogf.c %D%/clog10f.c \
	%D%/crealf.c %D%/ctanf.c %D%/cacoshf.c %D%/catanf.c \
	%D%/cephes_subrf.c %D%/conjf.c %D%/csinf.c %D%/ctanhf.c \
	%D%/cargf.c %D%/catanhf.c %D%/cexpf.c %D%/cpowf.c %D%/csinhf.c

libm_a_SOURCES += $(%C%_src) $(%C%_fsrc) $(%C%_lsrc)

LIBM_CHEWOUT_FILES += \
	%D%/cabs.def %D%/cacos.def %D%/cacosh.def %D%/carg.def \
	%D%/casin.def %D%/casinh.def %D%/catan.def %D%/catanh.def \
	%D%/ccos.def %D%/ccosh.def %D%/cexp.def %D%/cimag.def %D%/clog.def \
	%D%/clog10.def %D%/conj.def %D%/cpow.def %D%/cproj.def %D%/creal.def \
	%D%/csin.def %D%/csinh.def %D%/csqrt.def %D%/ctan.def %D%/ctanh.def

LIBM_CHAPTERS += %D%/complex.tex

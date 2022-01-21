info_TEXINFOS += %D%/libm.texi
%C%_libm_TEXINFOS = %D%/targetdep.tex $(LIBM_CHEWOUT_FILES)

LIBM_CHEWOUT_FILES =

LIBM_CHAPTERS =

%D%/libm.dvi: %D%/targetdep.tex $(LIBM_CHEWOUT_FILES)

%D%/targetdep.tex.stamp: $(LIBM_CHAPTERS)
	$(AM_V_GEN)cat $^ > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/targetdep.tex: %D%/targetdep.tex.stamp; @true

include %D%/common/Makefile.inc
include %D%/complex/Makefile.inc
include %D%/fenv/Makefile.inc
if NEWLIB_HW_FP
include %D%/mathfp/Makefile.inc
else
include %D%/math/Makefile.inc
endif

CLEANFILES += \
	%D%/targetdep.tex \
	%D%/targetdep.tex.stamp \
	$(LIBM_CHEWOUT_FILES)

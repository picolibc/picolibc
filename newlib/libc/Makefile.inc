info_TEXINFOS += %D%/libc.texi
%C%_libc_TEXINFOS = %D%/targetdep.tex $(LIBC_CHEWOUT_FILES)

LIBC_CHEWOUT_FILES =

LIBC_CHAPTERS = %D%/sys.tex

%D%/libc.dvi: %D%/targetdep.tex $(LIBC_CHEWOUT_FILES)

%D%/targetdep.tex.stamp: $(LIBC_CHAPTERS)
	$(AM_V_GEN)cat $^ > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/targetdep.tex: %D%/targetdep.tex.stamp; @true

include %D%/ctype/Makefile.inc
include %D%/iconv/Makefile.inc
include %D%/locale/Makefile.inc
include %D%/misc/Makefile.inc
include %D%/posix/Makefile.inc
include %D%/reent/Makefile.inc
include %D%/search/Makefile.inc
include %D%/signal/Makefile.inc
include %D%/ssp/Makefile.inc
include %D%/stdio64/Makefile.inc
include %D%/stdio/Makefile.inc
include %D%/stdlib/Makefile.inc
include %D%/string/Makefile.inc
include %D%/time/Makefile.inc

CLEANFILES += \
	%D%/targetdep.tex \
	%D%/targetdep.tex.stamp \
	$(LIBC_CHEWOUT_FILES)

info_TEXINFOS += %D%/libc.texi
%C%_libc_TEXINFOS = %D%/targetdep.tex $(LIBC_CHEWOUT_FILES)

LIBC_CHEWOUT_FILES =

LIBC_CHAPTERS = %D%/sys.tex
LIBC_DOCBOOK_OUT_FILES = $(LIBC_CHEWOUT_FILES:.def=.xml)

%D%/libc.dvi: %D%/targetdep.tex $(LIBC_CHEWOUT_FILES)

%D%/targetdep.tex.stamp: $(LIBC_CHAPTERS)
	$(AM_V_GEN)cat $^ > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/targetdep.tex: %D%/targetdep.tex.stamp; @true

%D%/libc.xml.stamp: %D%/libc.in.xml $(LIBC_CHAPTERS) $(LIBC_DOCBOOK_OUT_FILES)
	$(AM_V_at)\
	for chapter in $(LIBC_CHAPTERS); do \
	  $(TEXI2DOCBOOK) < $(srcdir)/$$chapter > %D%/`basename $${chapter%.tex}`.xml || exit 1; \
	done
	$(AM_V_GEN)xsltproc --xinclude --path $(builddir)/%D% --nonet $(srcdir)/refcontainers.xslt $< > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/libc.xml: %D%/libc.xml.stamp; @true

%C%_man: %D%/libc.xml
	$(AM_V_GEN)xmlto --skip-validation -o %D% --searchpath $(builddir)/%D% man -m $(srcdir)/man.xsl %D%/libc.xml
man: %C%_man

%C%_install-man: %C%_man
	$(MKDIR_P) $(DESTDIR)$(mandir)/man3
	$(INSTALL_DATA) %D%/*.3 $(DESTDIR)$(mandir)/man3/
install-man: %C%_install-man

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
	$(LIBC_CHEWOUT_FILES) \
	$(LIBC_DOCBOOK_OUT_FILES) \
	%D%/*.xml %D%/*.xml.stamp %D%/*.3

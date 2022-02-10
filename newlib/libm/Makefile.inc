info_TEXINFOS += %D%/libm.texi
%C%_libm_TEXINFOS = %D%/targetdep.tex $(LIBM_CHEWOUT_FILES)

LIBM_CHEWOUT_FILES =

LIBM_CHAPTERS =
LIBM_DOCBOOK_OUT_FILES = $(LIBM_CHEWOUT_FILES:.def=.xml)
$(LIBM_CHEWOUT_FILES): $(MKDOC)

%D%/libm.dvi: %D%/targetdep.tex $(LIBM_CHEWOUT_FILES)

%D%/targetdep.tex.stamp: $(LIBM_CHAPTERS)
	$(AM_V_GEN)cat $^ > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/targetdep.tex: %D%/targetdep.tex.stamp; @true

%D%/libm.xml.stamp: %D%/libm.in.xml $(LIBM_CHAPTERS) $(LIBM_DOCBOOK_OUT_FILES)
	$(AM_V_at)\
	for chapter in $(LIBM_CHAPTERS); do \
	  $(TEXI2DOCBOOK) < $(srcdir)/$$chapter > %D%/`basename $${chapter%.tex}`.xml || exit 1; \
	done
	$(AM_V_GEN)xsltproc --xinclude --path $(builddir)/%D% --nonet $(srcdir)/refcontainers.xslt $< > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $(@:.stamp=)
	$(AM_V_at)touch $@
%D%/libm.xml: %D%/libm.xml.stamp; @true

%C%_man: %D%/libm.xml
	$(AM_V_GEN)xmlto --skip-validation -o %D% --searchpath $(builddir)/%D% man -m $(srcdir)/man.xsl %D%/libm.xml
man: %C%_man

%C%_install-man: %C%_man
	$(MKDIR_P) $(DESTDIR)$(mandir)/man3
	$(INSTALL_DATA) %D%/*.3 $(DESTDIR)$(mandir)/man3/
install-man: %C%_install-man

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
	$(LIBM_CHEWOUT_FILES) \
	$(LIBM_DOCBOOK_OUT_FILES) \
	%D%/*.xml %D%/*.xml.stamp %D%/*.3

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

%D%/libm.xml: %D%/libm.in.xml $(LIBM_CHAPTERS) $(LIBM_DOCBOOK_OUT_FILES)
	$(AM_V_at)\
	for chapter in $(LIBM_CHAPTERS); do \
	  $(TEXI2DOCBOOK) < $(srcdir)/$$chapter > %D%/`basename $${chapter%.tex}`.xml || exit 1; \
	done
	$(AM_V_GEN)xsltproc --xinclude --path $(builddir)/%D% --nonet $(srcdir)/refcontainers.xslt $< > $@.tmp
	$(AM_V_at)$(SHELL) $(newlib_basedir)/../move-if-change $@.tmp $@

%C%_man.stamp: %D%/libm.xml
	$(AM_V_GEN)xmlto --skip-validation -o %D% --searchpath $(builddir)/%D% man -m $(srcdir)/man.xsl $(XMLTO_MAN_QUIET) %D%/libm.xml
	$(AM_V_at)touch $@
man: %C%_man.stamp

%C%_install-man: %C%_man.stamp
	$(MKDIR_P) $(DESTDIR)$(mandir)/man3
	$(INSTALL_DATA) %D%/*.3 $(DESTDIR)$(mandir)/man3/
install-man: %C%_install-man

## The order of includes is important for two reasons:
## * The integrated documentation (chapter ordering).
## * Object overridding -- machine dir must come last.
## Do not change the order without considering the doc impact.

if NEWLIB_HW_FP
include %D%/mathfp/Makefile.inc
else
include %D%/math/Makefile.inc
endif
include %D%/common/Makefile.inc
include %D%/complex/Makefile.inc
include %D%/fenv/Makefile.inc
include %D%/test/Makefile.inc

if HAVE_LIBM_MACHINE_AARCH64
include %D%/machine/aarch64/Makefile.inc
endif
if HAVE_LIBM_MACHINE_AMDGCN
include %D%/machine/amdgcn/Makefile.inc
endif
if HAVE_LIBM_MACHINE_ARM
include %D%/machine/arm/Makefile.inc
endif
if HAVE_LIBM_MACHINE_I386
include %D%/machine/i386/Makefile.inc
endif
if HAVE_LIBM_MACHINE_MIPS
include %D%/machine/mips/Makefile.inc
endif
if HAVE_LIBM_MACHINE_NDS32
include %D%/machine/nds32/Makefile.inc
endif
if HAVE_LIBM_MACHINE_POWERPC
include %D%/machine/powerpc/Makefile.inc
endif
if HAVE_LIBM_MACHINE_PRU
include %D%/machine/pru/Makefile.inc
endif
if HAVE_LIBM_MACHINE_SPARC
include %D%/machine/sparc/Makefile.inc
endif
if HAVE_LIBM_MACHINE_SPU
include %D%/machine/spu/Makefile.inc
endif
if HAVE_LIBM_MACHINE_RISCV
include %D%/machine/riscv/Makefile.inc
endif
if HAVE_LIBM_MACHINE_X86_64
include %D%/machine/x86_64/Makefile.inc
endif

CLEANFILES += \
	%D%/targetdep.tex \
	%D%/targetdep.tex.stamp \
	$(LIBM_CHEWOUT_FILES) \
	$(LIBM_DOCBOOK_OUT_FILES) \
	%D%/*.xml %C%_man.stamp %D%/*.3

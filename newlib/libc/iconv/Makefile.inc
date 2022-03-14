if ENABLE_NEWLIB_ICONV
cctdir = $(datadir)/iconv_data
cct_DATA = %D%/encoding.aliases
endif

LIBC_CHAPTERS += %D%/iconv.tex

## If mkdeps.pl or encodings.deps changes, make sure to regenerate outputs.
%C%_mkdeps_outputs = \
	$(top_srcdir)/iconv.m4 \
	$(srcdir)/%D%/ccs/ccsbi.c \
	$(srcdir)/%D%/ccs/ccsbi.h \
	$(srcdir)/%D%/ccs/ccsnames.h \
	$(srcdir)/%D%/ces/cesbi.c \
	$(srcdir)/%D%/ces/cesbi.h \
	$(srcdir)/%D%/ces/cesdeps.h \
	$(srcdir)/%D%/encoding.aliases \
	$(srcdir)/%D%/lib/aliasesbi.c \
	$(srcdir)/%D%/lib/encnames.h

$(%C%_mkdeps_outputs): @MAINTAINER_MODE_TRUE@ $(srcdir)/%D%/ces/mkdeps.pl $(srcdir)/%D%/lib/encoding.deps
	$(AM_V_GEN)cd $(srcdir)/%D%/ces && ./mkdeps.pl

noinst_DATA += $(%C%_mkdeps_outputs)

include %D%/ces/Makefile.inc
include %D%/ccs/Makefile.inc
include %D%/lib/Makefile.inc

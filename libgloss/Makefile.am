## Top level makefile for libgloss

ACLOCAL_AMFLAGS = -I . -I .. -I ../config

# Variables that will accumulate in subdirs.
info_TEXINFOS =
PHONY =

SUBDIRS = @subdirs@ .

tooldir = $(exec_prefix)/$(target_alias)

## These are roughly topologically sorted in order to make porting more
## streamlined.
FLAGS_TO_PASS = \
	"CC=$(CC)" \
	"CFLAGS=$(CFLAGS)" \
	"CFLAGS_FOR_TARGET=$(CFLAGS_FOR_TARGET)" \
	"CCASFLAGS=$(CCASFLAGS)" \
	"AR=$(AR)" \
	"RANLIB=$(RANLIB)" \
	"AR_FLAGS=$(AR_FLAGS)" \
	"MAKEINFO=$(MAKEINFO)" \
	"AS=$(AS)" \
	"LD=$(LD)" \
	"CROSS_CFLAGS=$(CROSS_CFLAGS)" \
	"TARGET_CFLAGS=$(TARGET_CFLAGS)" \
	"exec_prefix=$(exec_prefix)" \
	"prefix=$(prefix)" \
	"tooldir=$(tooldir)" \
	"infodir=$(infodir)" \
	"libdir=$(libdir)" \
	"top_toollibdir=$(toollibdir)" \
	"INSTALL=$(INSTALL)" \
	"INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
	"INSTALL_DATA=$(INSTALL_DATA)" \
	"DESTDIR=$(DESTDIR)"

include $(top_srcdir)/../multilib.am

TEXINFO_TEX = ../texinfo/texinfo.tex

if HAVE_DOC
include doc/Makefile.inc
endif

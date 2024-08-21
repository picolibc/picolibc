## Top level makefile for libgloss

ACLOCAL_AMFLAGS = -I . -I .. -I ../config

# Variables that will accumulate in subdirs.
bin_PROGRAMS =
check_PROGRAMS =
info_TEXINFOS =
CLEANFILES =
PHONY =

SUBDIRS = @subdirs@ .

srcroot = $(top_srcdir)/..
tooldir = $(exec_prefix)/$(target_alias)

multilibtooldir = $(tooldir)/lib$(MULTISUBDIR)
multilibtool_DATA =
multilibtool_LIBRARIES =
multilibtool_PROGRAMS =

includetooldir = $(tooldir)/include
includetool_DATA =

includesystooldir = $(tooldir)/include/sys
includesystool_DATA =

# These are useful for standalone object files like crt0.o.
AM_CFLAGS = $(AM_CFLAGS_$(subst /,_,$(@D))) $(AM_CFLAGS_$(subst /,_,$(@D)_$(<F)))
AM_CCASFLAGS = $(AM_CCASFLAGS_$(subst /,_,$(@D))) $(AM_CCASFLAGS_$(subst /,_,$(@D)_$(<F)))
AM_CPPFLAGS = $(AM_CPPFLAGS_$(subst /,_,$(@D))) $(AM_CPPFLAGS_$(subst /,_,$(@D)_$(<F)))
AM_LDFLAGS = $(AM_LDFLAGS_$(subst /,_,$(@D))) $(AM_LDFLAGS_$(subst -,_,$(subst /,_,$(@D)_$(@F))))

AM_CPPFLAGS += -idirafter $(srcroot)/include

# A fake library so automake will generate rules for plain objects that we want
# to install (e.g. our crt0.o objects).
noinst_LIBRARIES = libobjs.a
libobjs_a_SOURCES =

## Some convenience rules for debugging programs.
%.dis: %
	$(AM_V_GEN)$(OBJDUMP) -d $< > $@
%.bin: %
	$(AM_V_GEN)$(OBJCOPY) -O binary $< $@
%.hex: %
	$(AM_V_GEN)$(OBJCOPY) -O ihex $< $@
%.srec: %
	$(AM_V_GEN)$(OBJCOPY) -O srec $< $@

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
if CONFIG_AARCH64
include aarch64/Makefile.inc
endif
if CONFIG_ARC
include arc/Makefile.inc
endif
if CONFIG_ARC64
include arc64/Makefile.inc
endif
if CONFIG_ARM
include arm/Makefile.inc
endif
if CONFIG_BFIN
include bfin/Makefile.inc
endif
if CONFIG_CR16
include cr16/Makefile.inc
endif
if CONFIG_CSKY
include csky/Makefile.inc
endif
if CONFIG_D30V
include d30v/Makefile.inc
endif
if CONFIG_EPIPHANY
include epiphany/Makefile.inc
endif
if CONFIG_FR30
include fr30/Makefile.inc
endif
if CONFIG_FRV
include frv/Makefile.inc
endif
if CONFIG_FT32
include ft32/Makefile.inc
endif
if CONFIG_I386
include i386/Makefile.inc
endif
if CONFIG_I960
include i960/Makefile.inc
endif
if CONFIG_IQ2000
include iq2000/Makefile.inc
endif
if CONFIG_LIBNOSYS
include libnosys/Makefile.inc
endif
if CONFIG_LM32
include lm32/Makefile.inc
endif
if CONFIG_M32R
include m32r/Makefile.inc
endif
if CONFIG_MCORE
include mcore/Makefile.inc
endif
if CONFIG_MICROBLAZE
include microblaze/Makefile.inc
endif
if CONFIG_MN10200
include mn10200/Makefile.inc
endif
if CONFIG_MN10300
include mn10300/Makefile.inc
endif
if CONFIG_MOXIE
include moxie/Makefile.inc
endif
if CONFIG_MSP430
include msp430/Makefile.inc
endif
if CONFIG_NDS32
include nds32/Makefile.inc
endif
if CONFIG_NIOS2
include nios2/Makefile.inc
endif
if CONFIG_OR1K
include or1k/Makefile.inc
endif
if CONFIG_PRU
include pru/Makefile.inc
endif
if CONFIG_RISCV
include riscv/Makefile.inc
endif
if CONFIG_RL78
include rl78/Makefile.inc
endif
if CONFIG_RX
include rx/Makefile.inc
endif
if CONFIG_SPARC
include sparc/Makefile.inc
endif
if CONFIG_SPARC_LEON
include sparc_leon/Makefile.inc
endif
if CONFIG_TIC6X
include tic6x/Makefile.inc
endif
if CONFIG_V850
include v850/Makefile.inc
endif
if CONFIG_VISIUM
include visium/Makefile.inc
endif
if CONFIG_WINCE
include wince/Makefile.inc
endif
if CONFIG_XSTORMY16
include xstormy16/Makefile.inc
endif
if CONFIG_XTENSA
include xtensa/Makefile.inc
endif

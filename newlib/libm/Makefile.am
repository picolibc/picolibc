## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

if NEWLIB_HW_FP
SUBDIRS = mathfp common .
SUBLIBS = mathfp/lib.a common/lib.a
else
SUBDIRS = math common . 
SUBLIBS = math/lib.a common/lib.a
endif

noinst_LIBRARIES = libm.a

libm.a: $(SUBLIBS)
	rm -f $@
	rm -rf tmp
	mkdir tmp
	cd tmp; \
	  for i in $(SUBLIBS); do \
	    $(AR) x ../$$i; \
	done; \
	$(AR) $(AR_FLAGS) ../$@ *.o
	$(RANLIB) $@
	rm -rf tmp

$(SUBLIBS):

info_TEXINFOS = libm.texinfo

libm.info: targetdep.tex math/stmp-def

stmp-targetdep: force
	rm -f tmp.texi
	targetdoc=`pwd`/tmp.texi; \
	for d in $(SUBDIRS); do \
	  if test "$$d" != "."; then \
	    (cd $$d && $(MAKE) doc) || exit 1; \
	  fi; \
	done
	$(SHELL) $(newlib_basedir)/../move-if-change tmp.texi targetdep.tex
	touch $@

targetdep.tex: stmp-targetdep ; @true

math/stmp-def: stmp-targetdep ; @true

.PHONY: force
force:

CLEANFILES = tmp.texi targetdep.tex stmp-targetdep

ACLOCAL_AMFLAGS = -I ..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

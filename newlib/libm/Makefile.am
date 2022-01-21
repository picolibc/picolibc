## Process this file with automake to generate Makefile.in

if NEWLIB_HW_FP
MATHDIR = mathfp
else
MATHDIR = math
endif

SUBDIRS = $(MATHDIR) common complex fenv
if HAVE_LIBM_MACHINE_DIR
SUBDIRS += $(LIBM_MACHINE_DIR)
endif

libm_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
SUBLIBS = $(MATHDIR)/lib$(MATHDIR).$(aext) common/libcommon.$(aext) complex/libcomplex.$(aext) fenv/libfenv.$(aext) $(LIBM_MACHINE_LIB)
noinst_LTLIBRARIES = libm.la
libm_la_SOURCES =
libm_la_LIBADD = $(SUBLIBS)
else
SUBLIBS = $(MATHDIR)/lib.$(aext) common/lib.$(aext) complex/lib.$(aext) fenv/lib.$(aext) $(LIBM_MACHINE_LIB)
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
endif

$(SUBLIBS):

docbook-recursive: force
	for d in $(SUBDIRS); do \
	  if test "$$d" != "."; then \
	    (cd $$d && $(MAKE) docbook) || exit 1; \
	  fi; \
	done

math/stmp-xml complex/stmp-xml: docbook-recursive

man: math/stmp-xml complex/stmp-xml libm.in.xml
	xsltproc --xinclude --path ${builddir} --nonet ${srcdir}/../refcontainers.xslt ${srcdir}/libm.in.xml >libm.xml
	xmlto --skip-validation --searchpath ${builddir} man -m ${srcdir}/../man.xsl libm.xml

install-man: man
	mkdir -p $(DESTDIR)$(mandir)/man3
	$(INSTALL_DATA) *.3 $(DESTDIR)$(mandir)/man3/

.PHONY: force
force:

CLEANFILES = *.xml *.3

ACLOCAL_AMFLAGS = -I .. -I ../.. -I ../../config
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

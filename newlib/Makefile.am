## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

# Multilib support variables.
MULTISRCTOP =
MULTIBUILDTOP =
MULTIDIRS =
MULTISUBDIR =
MULTIDO = true
MULTICLEAN = true

# Work around what appears to be a GNU make bug handling MAKEFLAGS
# values defined in terms of make variables, as is the case for CC and
# friends when we are called from the top level Makefile.
AM_MAKEFLAGS = \
	"AR_FLAGS=$(AR_FLAGS)" \
	"CC_FOR_BUILD=$(CC_FOR_BUILD)" \
	"CFLAGS=$(CFLAGS)" \
	"CFLAGS_FOR_BUILD=$(CFLAGS_FOR_BUILD)" \
	"CFLAGS_FOR_TARGET=$(CFLAGS_FOR_TARGET)" \
	"INSTALL=$(INSTALL)" \
	"INSTALL_DATA=$(INSTALL_DATA)" \
	"INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
	"INSTALL_SCRIPT=$(INSTALL_SCRIPT)" \
	"LDFLAGS=$(LDFLAGS)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"LIBCFLAGS_FOR_TARGET=$(LIBCFLAGS_FOR_TARGET)" \
	"MAKE=$(MAKE)" \
	"MAKEINFO=$(MAKEINFO) $(MAKEINFOFLAGS)" \
	"PICFLAG=$(PICFLAG)" \
	"PICFLAG_FOR_TARGET=$(PICFLAG_FOR_TARGET)" \
	"SHELL=$(SHELL)" \
	"EXPECT=$(EXPECT)" \
	"RUNTEST=$(RUNTEST)" \
	"RUNTESTFLAGS=$(RUNTESTFLAGS)" \
	"exec_prefix=$(exec_prefix)" \
	"infodir=$(infodir)" \
	"libdir=$(libdir)" \
	"prefix=$(prefix)" \
	"tooldir=$(tooldir)" \
	"AR=$(AR)" \
	"AS=$(AS)" \
	"CC=$(CC)" \
	"LD=$(LD)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"NM=$(NM)" \
	"PICFLAG=$(PICFLAG)" \
	"RANLIB=$(RANLIB)" \
	"DESTDIR=$(DESTDIR)"

if HAVE_DOC
DOCDIR = doc
endif

SUBDIRS = libc libm $(DOCDIR) .

tooldir = $(exec_prefix)/$(host_alias)
toollibdir = $(tooldir)/lib$(MULTISUBDIR)

AR_FLAGS = rc

toollib_LIBRARIES = libm.a libc.a
toollib_DATA = $(CRT0)
noinst_DATA = stmp-targ-include

# The functions ldexp, frexp and modf are traditionally supplied in
# both libc.a and libm.a.  We build them in libm.a and copy them over,
# along with some required supporting routines.
MATHOBJS_IN_LIBC = \
	s_isinf.o sf_isinf.o \
	s_isnan.o sf_isnan.o \
	s_ldexp.o sf_ldexp.o \
	s_frexp.o sf_frexp.o s_modf.o sf_modf.o \
	s_scalbn.o sf_scalbn.o s_finite.o sf_finite.o \
	s_copysign.o sf_copysign.o s_infconst.o

libc.a: libc/libc.a libm.a
	rm -rf libc.a libg.a tmp
	mkdir tmp
	cd tmp; \
	 $(AR) x ../libm.a $(MATHOBJS_IN_LIBC) ; \
	 $(AR) x ../libc/libc.a ; \
	 $(AR) $(AR_FLAGS) ../$@ *.o
	$(RANLIB) libc.a
	ln libc.a libg.a
	rm -rf tmp

libc/libc.a: ; @true

libm.a: libm/libm.a
	rm -f $@
	ln libm/libm.a $@ >/dev/null 2>/dev/null || cp libm/libm.a $@

libm/libm.a: ; @true

crt0.o: $(CRT0_DIR)/$(CRT0)
	rm -f $@
	ln $(CRT0_DIR)/$(CRT0) $@ >/dev/null 2>/dev/null \
	 || cp $(CRT0_DIR)/$(CRT0) $@

$(CRT0_DIR)/$(CRT0): ; @true

all-recursive: stmp-targ-include

# The targ-include directory just holds the includes files for the
# particular system and machine we have been configured for.  It is
# used while building.
stmp-targ-include: config.status
	-rm -rf targ-include stmp-targ-include
	mkdir targ-include targ-include/sys targ-include/machine
	-for i in $(srcdir)/libc/machine/$(machine_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/machine/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/sys/`basename $$i`; \
	    else true; fi ; \
	  done
	touch $@

CLEANFILES = targ-include stmp-targ-include

install-data-local:	install-toollibLIBRARIES
	rm -f $(DESTDIR)$(toollibdir)/libg.a
	ln $(DESTDIR)$(toollibdir)/libc.a $(DESTDIR)$(toollibdir)/libg.a
	$(MULTIDO) $(AM_MAKEFLAGS) DO=install multi-do
	-if [ -z "$(MULTISUBDIR)" ]; then \
	  for i in $(srcdir)/libc/include/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	  done; \
	  for i in $(srcdir)/libc/include/machine/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	  done; \
	  for i in $(srcdir)/libc/machine/$(machine_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	    else true; fi ; \
	  done; \
	  for i in $(srcdir)/libc/include/sys/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	  done; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	else true; fi

# Force makedoc to be built before building info files.
info-recursive: doc/makedoc
dvi-recursive: doc/makedoc
doc/makedoc:
	cd doc && $(MAKE) all

CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

# Multilib support.
.PHONY: all-multi mostlyclean-multi clean-multi distclean-multi \
	maintainer-clean-multi

all-recursive: all-multi
mostlyclean-recursive: mostlyclean-multi
clean-recursive: clean-multi
distclean-recursive: distclean-multi
maintainer-clean-recursive: maintainer-clean-multi

all-multi:
	$(MULTIDO) $(AM_MAKEFLAGS) DO=all multi-do
mostlyclean-multi:
	$(MULTICLEAN) $(AM_MAKEFLAGS) DO=mostlyclean multi-clean
clean-multi:
	$(MULTICLEAN) $(AM_MAKEFLAGS) DO=clean multi-clean
distclean-multi:
	$(MULTICLEAN) $(AM_MAKEFLAGS) DO=distclean multi-clean
maintainer-clean-multi:
	$(MULTICLEAN) $(AM_MAKEFLAGS) DO=maintainer-clean multi-clean

MAKEOVERRIDES=

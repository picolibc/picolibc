## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dejagnu
ACLOCAL_AMFLAGS = -I . -I .. -I ../config

# Variables that will accumulate in subdirs.
CLEANFILES =
PHONY =
SUFFIXES =
info_TEXINFOS =
noinst_DATA =

# The newlib hardware floating-point routines have been disabled due to
# inaccuracy.  If you wish to work on them, you will need to edit the
# configure.in file to re-enable the configuration option.  By default,
# the NEWLIB_HW_FP variable will always be false.
if NEWLIB_HW_FP
MATHDIR = mathfp
else
MATHDIR = math
endif

# Work around what appears to be a GNU make bug handling MAKEFLAGS
# values defined in terms of make variables, as is the case for CC and
# friends when we are called from the top level Makefile.
AM_MAKEFLAGS = \
	"AR_FLAGS=$(AR_FLAGS)" \
	"CC_FOR_BUILD=$(CC_FOR_BUILD)" \
	"CFLAGS=$(CFLAGS)" \
	"CCASFLAGS=$(CCASFLAGS)" \
	"CFLAGS_FOR_BUILD=$(CFLAGS_FOR_BUILD)" \
	"CFLAGS_FOR_TARGET=$(CFLAGS_FOR_TARGET)" \
	"INSTALL=$(INSTALL)" \
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
	"top_toollibdir=$(toollibdir)" \
	"AR=$(AR)" \
	"AS=$(AS)" \
	"CC=$(CC_FOR_NEWLIB)" \
	"LD=$(LD)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"NM=$(NM)" \
	"PICFLAG=$(PICFLAG)" \
	"RANLIB=$(RANLIB)" \
	"DESTDIR=$(DESTDIR)"

FLAGS_TO_PASS=$(AM_MAKEFLAGS)

SUBDIRS = libc libm .

tooldir = $(exec_prefix)/$(host_alias)
toollibdir = $(tooldir)/lib$(MULTISUBDIR)

AR_FLAGS = rc

noinst_DATA += stmp-targ-include

toollib_LIBRARIES = libm.a \
	libc.a
noinst_DATA += libg.a
if HAVE_MULTISUBDIR
BUILD_MULTISUBDIR = $(builddir)$(MULTISUBDIR)
if HAVE_CRT0
noinst_DATA += $(BUILD_MULTISUBDIR)/$(CRT0)
endif
noinst_DATA += $(BUILD_MULTISUBDIR)/libc.a $(BUILD_MULTISUBDIR)/libm.a
endif

toollib_DATA = $(CRT0) $(CRT1)

CLEANFILES += libg.a

# The functions ldexp, frexp and modf are traditionally supplied in
# both libc.a and libm.a.  We build them in libm.a and copy them over,
# along with some required supporting routines.
MATHOBJS_IN_LIBC = \
	$(lpfx)s_fpclassify.o $(lpfx)sf_fpclassify.o \
	$(lpfx)s_isinf.o $(lpfx)sf_isinf.o \
	$(lpfx)s_isnan.o $(lpfx)sf_isnan.o \
	$(lpfx)s_isinfd.o $(lpfx)sf_isinff.o \
	$(lpfx)s_isnand.o $(lpfx)sf_isnanf.o \
	$(lpfx)s_nan.o $(lpfx)sf_nan.o \
	$(lpfx)s_ldexp.o $(lpfx)sf_ldexp.o \
	$(lpfx)s_frexp.o $(lpfx)sf_frexp.o $(lpfx)frexpl.o \
	$(lpfx)s_modf.o \
	$(lpfx)sf_modf.o $(lpfx)s_scalbn.o \
	$(lpfx)sf_scalbn.o \
	$(lpfx)s_finite.o $(lpfx)sf_finite.o \
	$(lpfx)s_copysign.o $(lpfx)sf_copysign.o

libc.a: libc/libc.a libm.a
	rm -rf libc.a tmp
	mkdir tmp
	cd tmp; \
	 $(AR) x ../libm.a $(MATHOBJS_IN_LIBC) ; \
	 $(AR) x ../libc/libc.a ; \
	 $(AR) $(AR_FLAGS) ../$@ *.o
	$(RANLIB) libc.a
	rm -rf tmp

libc/libc.a: ; @true

libg.a: libc.a
	$(AM_V_GEN)ln libc.a libg.a >/dev/null 2>/dev/null || cp libc.a libg.a

libm.a: libm/libm.a
	rm -f $@
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@

libm/libm.a: ; @true

if HAVE_MULTISUBDIR
$(BUILD_MULTISUBDIR):
	$(MKDIR_P) $@

$(BUILD_MULTISUBDIR)/crt0.o: $(CRT0_DIR)$(CRT0) $(BUILD_MULTISUBDIR)
	rm -f $@
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(BUILD_MULTISUBDIR)/libc.a: libc.a $(BUILD_MULTISUBDIR)
	rm -f $@ $(BUILD_MULTISUBDIR)/libg.a
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@
	ln libg.a $(BUILD_MULTISUBDIR)/libg.a >/dev/null 2>/dev/null || \
	cp libg.a $(BUILD_MULTISUBDIR)/libg.a

$(BUILD_MULTISUBDIR)/libm.a: libm.a $(BUILD_MULTISUBDIR)
	rm -f $@
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@
endif

crt0.o: $(CRT0_DIR)$(CRT0)
	rm -f $@
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(CRT0_DIR)$(CRT0): ; @true

crt1.o: $(CRT1_DIR)$(CRT1)
	rm -f $@
	ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(CRT1_DIR)$(CRT1): ; @true

targ-include:
	mkdir $@

targ-include/newlib.h: targ-include/_newlib_version.h

targ-include/sys targ-include/machine targ-include/bits: targ-include/_newlib_version.h targ-include/newlib.h
	mkdir $@

targ-include/newlib.h: newlib.h targ-include
	cp newlib.h $@

targ-include/_newlib_version.h: _newlib_version.h targ-include
	cp _newlib_version.h $@

all-recursive: stmp-targ-include

# The targ-include directory just holds the includes files for the
# particular system and machine we have been configured for.  It is
# used while building.
stmp-targ-include: config.status
	-rm -rf targ-include stmp-targ-include
	$(MAKE) targ-include/sys \
		targ-include/machine \
		targ-include/bits
	-if [ -n "$(shared_machine_dir)" ]; then \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/machine/*.h; do \
		if [ -f $$i ]; then \
		  cp $$i targ-include/machine/`basename $$i`; \
		else true; fi ; \
	      done; \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/sys/*.h; do \
		if [ -f $$i ]; then \
		  cp $$i targ-include/sys/`basename $$i`; \
		else true; fi ; \
	      done; \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/include/*.h; do \
		if [ -f $$i ]; then \
		  cp $$i targ-include/`basename $$i`; \
		else true; fi ; \
	      done; \
	  fi
	-for i in $(srcdir)/libc/machine/$(machine_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/machine/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/machine/$(machine_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/sys/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/machine/$(machine_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/include/*; do \
	    if [ -d $$i ]; then \
		for j in $$i/*.h; do \
		    if [ ! -d targ-include/`basename $$i` ]; then \
		    	mkdir targ-include/`basename $$i`; \
		    fi; \
	      	    cp $$j targ-include/`basename $$i`/`basename $$j`; \
		done \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/sys/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/bits/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/bits/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/machine/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/machine/$(machine_dir)/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/machine/`basename $$i`; \
	    else true; fi ; \
	  done
	-for i in $(srcdir)/libc/sys/$(sys_dir)/machine/$(machine_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	      cp $$i targ-include/machine/`basename $$i`; \
	    else true; fi ; \
	  done
	$(MAKE) targ-include/newlib.h
	touch $@

CLEANFILES += stmp-targ-include

## We hook install-multi because this Makefile doesn't have any exec targets,
## only data targets.  If that ever changes, this should be removed and the
## install-exec-local in ../multilib.am will kick in.
install-data-local: install-toollibLIBRARIES install-multi
	rm -f $(DESTDIR)$(toollibdir)/libg.a
	ln $(DESTDIR)$(toollibdir)/libc.a $(DESTDIR)$(toollibdir)/libg.a >/dev/null 2>/dev/null || cp $(DESTDIR)$(toollibdir)/libc.a $(DESTDIR)$(toollibdir)/libg.a
	-if [ -z "$(MULTISUBDIR)" ]; then \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include; \
	  for i in $(srcdir)/libc/include/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	  done; \
	  $(INSTALL_DATA) newlib.h $(DESTDIR)$(tooldir)/include/newlib.h; \
	  $(INSTALL_DATA) _newlib_version.h $(DESTDIR)$(tooldir)/include/_newlib_version.h; \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include/machine; \
	  for i in $(srcdir)/libc/include/machine/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	  done; \
	  if [ -n "$(shared_machine_dir)" ]; then \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/machine/*.h; do \
	      if [ -f $$i ]; then \
	       $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	      else true; fi ; \
	    done; \
	  fi ; \
	  for i in $(srcdir)/libc/machine/$(machine_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	    else true; fi ; \
	  done; \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include/rpc; \
	  for i in $(srcdir)/libc/include/rpc/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/rpc/`basename $$i`; \
	  done; \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include/ssp; \
	  for i in $(srcdir)/libc/include/ssp/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/ssp/`basename $$i`; \
	  done; \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include/sys; \
	  for i in $(srcdir)/libc/include/sys/*.h; do \
	   $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	  done; \
	  if [ -n "$(shared_machine_dir)" ]; then \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/sys/*.h; do \
	      if [ -f $$i ]; then \
	       $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	      else true; fi ; \
	    done ; \
	    for i in $(srcdir)/libc/machine/$(shared_machine_dir)/include/*.h; do \
	      if [ -f $$i ]; then \
	       $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	      else true; fi ; \
	    done ; \
	  fi ; \
	  for i in $(srcdir)/libc/machine/$(machine_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/machine/$(machine_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/sys/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/sys/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  $(mkinstalldirs) $(DESTDIR)$(tooldir)/include/bits; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/bits/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/bits/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/machine/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/include/*; do \
	    if [ -d $$i ]; then \
	    $(MKDIR_P) $(DESTDIR)$(tooldir)/include/`basename $$i`; \
		for j in $$i/*.h; do \
	            $(INSTALL_DATA) $$j $(DESTDIR)$(tooldir)/include/`basename $$i`/`basename $$j`; \
		done ; \
	    else true; fi ; \
	  done ; \
	  for i in $(srcdir)/libc/sys/$(sys_dir)/machine/$(machine_dir)/include/*.h; do \
	    if [ -f $$i ]; then \
	     $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/machine/`basename $$i`; \
	    else true; fi ; \
	  done ; \
	  for i in $(NO_INCLUDE_LIST); do \
	    if [ -f $(DESTDIR)$(tooldir)/include/$$i ]; then \
		rm $(DESTDIR)$(tooldir)/include/$$i; \
	    else true; fi ; \
	  done ; \
	else true; fi

# Generate Unicode data tables for libc/string/wcwidth and libc/ctype/??w*
unidata:
	cd $(srcdir)/libc/string; ./mkunidata -u
	cd $(srcdir)/libc/ctype; ./mkunidata -u

# Force makedoc to be built before building info files.
info-recursive dvi-recursive: doc/makedoc$(EXEEXT_FOR_BUILD)

CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

include ../multilib.am

check-multi:
	$(MULTIDO) $(AM_MAKEFLAGS) DO=check multi-do # $(MAKE)

MAKEOVERRIDES=

# dejagnu support

RUNTESTFLAGS=

site.exp: Makefile
	@echo 'Making a new site.exp file...'
	@test ! -f site.bak || rm -f site.bak
	@echo '## these variables are automatically generated by make ##' > $@-t
	@echo '# Do not edit here.  If you wish to override these values' >> $@-t
	@echo '# edit the last section' >> $@-t
	@echo 'set tool $(DEJATOOL)' >> $@-t
	@echo 'set tool_version $(VERSION)' >> $@-t
	@echo 'set srcdir $(srcdir)/testsuite' >> $@-t
	@echo 'set objdir' `pwd` >> $@-t
	@echo 'set tmpdir' `pwd`/testsuite >> $@-t
	@echo 'set multibuildtop ./$(MULTIBUILDTOP)' >> $@-t
	@echo 'set host_alias $(build_alias)' >> $@-t
	@echo 'set host_triplet $(build_triplet)' >> $@-t
	@echo 'set target_alias $(host_alias)' >> $@-t
	@echo 'set target_triplet $(host_triplet)' >> $@-t
	@echo '## All variables above are generated by configure. Do Not Edit ##' >> $@-t
	@test ! -f site.exp || sed '1,/^## All variables above are.*##/ d' site.exp >> $@-t
	@test ! -f site.exp || mv site.exp site.bak
	@mv $@-t site.exp

check-DEJAGNU: site.exp
	@if [ -d testsuite ] ; then \
	  true ; \
	else \
	  mkdir testsuite ; \
	fi
	@rm -f testsuite/site.exp
	@cp site.exp testsuite/site.exp
	@rootme=`pwd` ; \
	export rootme ; \
	srcdir=`cd ${srcdir}; pwd` ; \
	export srcdir ; \
	EXPECT=`if [ -f $${rootme}/${MULTIBUILDTOP}../../expect/expect ] ; then \
	          echo $${rootme}/${MULTIBUILDTOP}../../expect/expect ; \
	        else \
	          echo expect ; \
	        fi` ; \
	export EXPECT ; \
	echo "Found expect: $${EXPECT}" ; \
	if [ -f $(top_builddir)/$(MULTIBUILDTOP)../../expect/expect ] ; then \
	  TCL_LIBRARY=`cd $(top_srcdir)/../tcl/library && pwd` ; \
	  export TCL_LIBRARY ; \
	  echo "Found TCL library directory: $${TCL_LIBRARY}" ; \
	fi ; \
	RUNTEST=`if [ -f $${srcdir}/../dejagnu/runtest ] ; then \
	           echo $${srcdir}/../dejagnu/runtest ; \
	         else \
	           echo runtest ; \
	         fi` ; \
	export RUNTEST ; \
	echo "Found runtest: $${RUNTEST}" ; \
	cd testsuite ; \
	if $(SHELL) -c "$$RUNTEST --version" > /dev/null 2>&1 ; then \
	   $$RUNTEST --tool $(DEJATOOL) --srcdir $${srcdir}/testsuite $(RUNTESTFLAGS) ; \
	   : ; \
	else \
	   echo "WARNING: could not find \`runtest'" 1>&2 ; \
	fi

clean-local:
	-rm -rf targ-include newlib.h _newlib_version.h stamp-*

# Workaround bad automake/texinfo interactions.
# https://bugs.gnu.org/23599
TEXI2DVI = texi2dvi -E

TEXINFO_TEX = ../texinfo/texinfo.tex

if HAVE_DOC
include doc/Makefile.inc
endif
include libc/Makefile.inc
include libm/Makefile.inc

.PHONY: $(PHONY)

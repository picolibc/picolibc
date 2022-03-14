## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dejagnu
ACLOCAL_AMFLAGS = -I . -I .. -I ../config

# Variables that will accumulate in subdirs.
CLEANFILES =
EXTRA_DIST =
EXTRA_PROGRAMS =
PHONY =
SUFFIXES =
check_PROGRAMS =
info_TEXINFOS =
noinst_DATA =
INSTALL_DATA_LOCAL =

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

tooldir = $(exec_prefix)/$(host_alias)
toollibdir = $(tooldir)/lib$(MULTISUBDIR)

# These are useful for standalone object files like crt0.o.
AM_CFLAGS = $(AM_CFLAGS_$(subst /,_,$(@D))) $(AM_CFLAGS_$(subst /,_,$(@D)_$(<F)))
AM_CCASFLAGS = $(AM_CCASFLAGS_$(subst /,_,$(@D))) $(AM_CCASFLAGS_$(subst /,_,$(@D)_$(<F)))
AM_CPPFLAGS = $(NEWLIB_CFLAGS) $(TARGET_CFLAGS) $(AM_CPPFLAGS_$(subst /,_,$(@D))) $(AM_CPPFLAGS_$(subst /,_,$(@D)_$(<F)))

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

## GNU ar has undocumented behavior when specifying the same name multiple times
## in a single invocation, so we have to dedupe ourselves.  The algorithm here:
## - Generates the set of unique objects based on the basename.
## - Favors objects later in the list (since machine objects come last).
## - Outputs object list in same order as input for reproducibility.
## https://sourceware.org/PR28917
AWK_UNIQUE_OBJS = $(AWK) '{ \
  for (i = NF; i > 0; --i) { \
    count = split($$i, parts, "/"); \
    name = parts[count]; \
    if (!(name in seen)) { \
      objs[i] = $$i; \
      seen[name] = 1; \
    } \
  } \
  for (i in objs) \
    print objs[i]; \
}'

# The functions ldexp, frexp and modf are traditionally supplied in
# both libc.a and libm.a.  We build them in libm.a and copy them over,
# along with some required supporting routines.
MATHOBJS_IN_LIBC = \
	libm_a-s_fpclassify.o libm_a-sf_fpclassify.o \
	libm_a-s_isinf.o libm_a-sf_isinf.o \
	libm_a-s_isnan.o libm_a-sf_isnan.o \
	libm_a-s_isinfd.o libm_a-sf_isinff.o \
	libm_a-s_isnand.o libm_a-sf_isnanf.o \
	libm_a-s_nan.o libm_a-sf_nan.o \
	libm_a-s_ldexp.o libm_a-sf_ldexp.o \
	libm_a-s_frexp.o libm_a-sf_frexp.o libm_a-frexpl.o \
	libm_a-s_modf.o \
	libm_a-sf_modf.o libm_a-s_scalbn.o \
	libm_a-sf_scalbn.o \
	libm_a-s_finite.o libm_a-sf_finite.o \
	libm_a-s_copysign.o libm_a-sf_copysign.o

## Place the libm_a objects in libc/ to keep the top dir tidy.
stamp-libc-math-objects: libm.a
	$(AM_V_GEN)cd libc && $(AR) x ../$< $(MATHOBJS_IN_LIBC)
	$(AM_V_at)touch $@

CLEANFILES += $(MATHOBJS_IN_LIBC)

libc_a_SOURCES =
libc_a_CFLAGS = $(AM_CFLAGS) $(libc_a_CFLAGS_$(subst /,_,$(@D))) $(libc_a_CFLAGS_$(subst /,_,$(@D)_$(<F)))
libc_a_CCASFLAGS = $(AM_CCASFLAGS) $(libc_a_CCASFLAGS_$(subst /,_,$(@D))) $(libc_a_CCASFLAGS_$(subst /,_,$(@D)_$(<F)))
libc_a_CPPFLAGS = $(AM_CPPFLAGS) $(libc_a_CPPFLAGS_$(subst /,_,$(@D))) $(libc_a_CPPFLAGS_$(subst /,_,$(@D)_$(<F)))
libc_a_DEPENDENCIES = stamp-libc-math-objects
## Best to avoid libc_a_LIBADD entirely to avoid having 2 independent lists
## with separate overriding behavior (libc_a_OBJECTS is the other).  See the
## AWK_UNIQUE_OBJS comment aove for more details.
$(libc_a_OBJECTS) $(libc_a_LIBADD): stmp-targ-include

libc.a: $(libc_a_OBJECTS) $(libc_a_DEPENDENCIES)
	$(AM_V_at)rm -f $@
	$(AM_V_AR)objs=`echo $(libc_a_OBJECTS) | $(AWK_UNIQUE_OBJS)` || exit $$?; \
	  $(AR) $(ARFLAGS) $@ $$objs $(MATHOBJS_IN_LIBC:%=libc/%)
	$(AM_V_at)$(RANLIB) $@

libg.a: libc.a
	$(AM_V_GEN)ln libc.a libg.a >/dev/null 2>/dev/null || cp libc.a libg.a

libm_a_SOURCES =
libm_a_CFLAGS = $(AM_CFLAGS) $(libm_a_CFLAGS_$(subst /,_,$(@D))) $(libm_a_CFLAGS_$(subst /,_,$(@D)_$(<F)))
libm_a_CCASFLAGS = $(AM_CCASFLAGS) $(libm_a_CCASFLAGS_$(subst /,_,$(@D))) $(libm_a_CCASFLAGS_$(subst /,_,$(@D)_$(<F)))
libm_a_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/libm/common $(libm_a_CPPFLAGS_$(subst /,_,$(@D))) $(libm_a_CPPFLAGS_$(subst /,_,$(@D)_$(<F)))
## Best to avoid libm_a_LIBADD entirely to avoid having 2 independent lists
## with separate overriding behavior (libm_a_OBJECTS is the other).  See the
## AWK_UNIQUE_OBJS comment aove for more details.
$(libm_a_OBJECTS): stmp-targ-include

libm.a: $(libm_a_OBJECTS) $(libm_a_DEPENDENCIES)
	$(AM_V_at)rm -f $@
	$(AM_V_AR)objs=`echo $(libm_a_OBJECTS) | $(AWK_UNIQUE_OBJS)` || exit $$?; \
	  $(AR) $(ARFLAGS) $@ $$objs
	$(AM_V_at)$(RANLIB) $@

if HAVE_MULTISUBDIR
$(BUILD_MULTISUBDIR):
	$(AM_V_at)$(MKDIR_P) $@

$(BUILD_MULTISUBDIR)/crt0.o: $(CRT0_DIR)$(CRT0) $(BUILD_MULTISUBDIR)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(BUILD_MULTISUBDIR)/libc.a: libc.a $(BUILD_MULTISUBDIR)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(BUILD_MULTISUBDIR)/libg.a: libg.a ${BUILD_MULTISUBDIR}
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(BUILD_MULTISUBDIR)/libm.a: libm.a $(BUILD_MULTISUBDIR)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@
endif

crt0.o: $(CRT0_DIR)$(CRT0)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(CRT0_DIR)$(CRT0): stmp-targ-include

crt1.o: $(CRT1_DIR)$(CRT1)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@

$(CRT1_DIR)$(CRT1): stmp-targ-include

targ-include:
	$(AM_V_GEN)$(MKDIR_P) $@

targ-include/newlib.h: targ-include/_newlib_version.h

targ-include/sys targ-include/machine targ-include/bits: targ-include/_newlib_version.h targ-include/newlib.h
	$(AM_V_GEN)$(MKDIR_P) $@

targ-include/newlib.h: newlib.h targ-include
	$(AM_V_GEN)cp newlib.h $@

targ-include/_newlib_version.h: _newlib_version.h targ-include
	$(AM_V_GEN)cp _newlib_version.h $@

# The targ-include directory just holds the includes files for the
# particular system and machine we have been configured for.  It is
# used while building.
TARG_INCLUDE_CP_DIR = \
	; srcdir="$(srcdir)/libc/$$s"; dstdir="targ-include/$$d"; \
	if (cd $$srcdir 2>/dev/null && [ "`echo *.h`" != "*.h" ]); then cp $$srcdir/*.h $$dstdir/; fi
stmp-targ-include: config.status
	$(AM_V_GEN)rm -rf targ-include stmp-targ-include
	$(AM_V_at)$(MAKE) targ-include/sys \
		targ-include/machine \
		targ-include/bits
	$(AM_V_at)if [ -n "$(shared_machine_dir)" ]; then \
	    s=machine/$(shared_machine_dir)/machine d=machine $(TARG_INCLUDE_CP_DIR) || exit $$?; \
	    s=machine/$(shared_machine_dir)/sys d=sys $(TARG_INCLUDE_CP_DIR) || exit $$?; \
	    s=machine/$(shared_machine_dir)/include d= $(TARG_INCLUDE_CP_DIR) || exit $$?; \
	  fi
	$(AM_V_at)s=machine/$(machine_dir)/machine d=machine $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=machine/$(machine_dir)/sys d=sys $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=machine/$(machine_dir)/include d= $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=sys/$(sys_dir)/include d= $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)for i in $(srcdir)/libc/sys/$(sys_dir)/include/*/; do \
	    if [ -d $$i ]; then \
		d=`basename $$i`; \
		$(MKDIR_P) targ-include/$$d; \
		s=sys/${sys_dir}/include/$$d $(TARG_INCLUDE_CP_DIR) || exit $$?; \
	    fi \
	  done
	$(AM_V_at)s=sys/$(sys_dir)/sys d=sys $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=sys/$(sys_dir)/bits d=bits $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=sys/$(sys_dir)/machine d=machine $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=sys/$(sys_dir)/machine/$(machine_dir) d=machine $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)s=sys/$(sys_dir)/machine/$(machine_dir)/include d=machine $(TARG_INCLUDE_CP_DIR)
	$(AM_V_at)$(MAKE) targ-include/newlib.h
	$(AM_V_at)touch $@

CLEANFILES += stmp-targ-include

## We hook install-multi because this Makefile doesn't have any exec targets,
## only data targets.  If that ever changes, this should be removed and the
## install-exec-local in ../multilib.am will kick in.
install-data-local: install-toollibLIBRARIES install-multi $(INSTALL_DATA_LOCAL)
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

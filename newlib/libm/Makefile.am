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

$(SUBLIBS):

ACLOCAL_AMFLAGS = -I .. -I ../.. -I ../../config
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

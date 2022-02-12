%C%_CPPFLAGS = -I$(srcdir)/%D%/include

# Here is all of the simulator stuff.
multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/clear_cache_range.c \
	%D%/syscalls.c
%C%_libsim_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(%C%_CPPFLAGS)

check_PROGRAMS += %D%/sim-test

# Here is all of the development board stuff.
multilibtool_DATA += \
	%D%/crt0.o \
	%D%/basiccrt.o \
	%D%/basiccrts.o \
	%D%/basiccrt561.o \
	%D%/basiccrt561s.o \
	%D%/basiccrt561b.o
# 	%D%/basiccrt60x.o \
#	%D%/basiccrt60xs.o \
#	%D%/basiccrt60xc1.o
libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/basiccrt.S

if !HAVE_MULTISUBDIR
multilibtool_DATA += \
	%D%/bfin-common-sc.ld %D%/bfin-common-mc.ld %D%/bfin-common-mc0.ld \
	%D%/bf504.ld %D%/bf506.ld \
	%D%/bf512.ld %D%/bf514.ld %D%/bf516.ld %D%/bf518.ld \
	%D%/bf522.ld %D%/bf523.ld %D%/bf524.ld %D%/bf525.ld %D%/bf526.ld %D%/bf527.ld \
	%D%/bf531.ld %D%/bf532.ld %D%/bf533.ld \
	%D%/bf534.ld %D%/bf536.ld %D%/bf537.ld \
	%D%/bf538.ld %D%/bf539.ld \
	%D%/bf542.ld %D%/bf544.ld %D%/bf547.ld %D%/bf548.ld %D%/bf549.ld \
	%D%/bf561.ld %D%/bf561a.ld %D%/bf561b.ld %D%/bf561m.ld \
	%D%/bf606.ld %D%/bf606c0.ld %D%/bf606c1.ld %D%/bf606m.ld \
	%D%/bf607.ld %D%/bf607c0.ld %D%/bf607c1.ld %D%/bf607m.ld \
	%D%/bf608.ld %D%/bf608c0.ld %D%/bf608c1.ld %D%/bf608m.ld \
	%D%/bf609.ld %D%/bf609c0.ld %D%/bf609c1.ld %D%/bf609m.ld \
	%D%/bf592.ld

includetool_DATA += $(wildcard $(srcdir)/%D%/include/*.h)
includesystool_DATA += $(wildcard $(srcdir)/%D%/include/sys/*.h)
endif

multilibtool_LIBRARIES += %D%/libbfinbsp.a
%C%_libbfinbsp_a_SOURCES = \
	%D%/clear_cache_range.c \
	%D%/_exit.c
%C%_libbfinbsp_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(%C%_CPPFLAGS)

%D%/basiccrt.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),,-mcpu=bf532-any)
%D%/basiccrts.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),,-mcpu=bf532-any) -D__BFIN_SDRAM
%D%/basiccrt561.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf561-any,-mcpu=bf561-none)
%D%/basiccrt561s.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf561-any,-mcpu=bf561-none) -D__BFIN_SDRAM
%D%/basiccrt561b.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf561-any,-mcpu=bf561-none) -D__ADSPBF561_COREB__
%D%/basiccrt60x.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf608-any,-mcpu=bf608-none)
%D%/basiccrt60xs.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf608-any,-mcpu=bf608-none) -D__BFIN_SDRAM
%D%/basiccrt60xc1.$(OBJEXT): %D%/basiccrt.S
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -o $@ -c $< $(if $(findstring mcpu=,$(CFLAGS)),-mcpu=bf608-any,-mcpu=bf608-none) -D__ADSPBF60x_CORE1__

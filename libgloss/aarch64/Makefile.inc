multilibtool_LIBRARIES += %D%/librdimon.a
%C%_librdimon_a_SOURCES = \
	%D%/_exit.c \
	%D%/_kill.c \
	%D%/ftruncate.c \
	%D%/libcfunc.c \
	%D%/syscalls.c \
	%D%/truncate.c
%C%_librdimon_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DSEMIHOST_V2 -DARM_RDI_MONITOR

%C%_SPECS = \
	%D%/aem-v8-r.specs \
	%D%/aem-validation.specs \
	%D%/aem-ve.specs \
	%D%/rdimon.specs
multilibtool_DATA += \
	%D%/crt0.o \
	%D%/rdimon-crt0.o \
	$(%C%_SPECS)

%D%/crt0.$(OBJEXT): %D%/crt0.S
	$(AM_V_CPPAS)$(CPPASCOMPILE) -DSEMIHOST_V2 -o $@ -c $<
%D%/rdimon-crt0.$(OBJEXT): %D%/crt0.S
	$(AM_V_CPPAS)$(CPPASCOMPILE) -DSEMIHOST_V2 -DARM_RDI_MONITOR -o $@ -c $<

## Make copies of the scripts to facilitate in-tree bootstrapping.
%D%/%.specs: %D%/$(AARCH64_OBJTYPE)%.specs
	$(AM_V_GEN)cp $< $@

CLEANFILES += $(%C%_SPECS)

include %D%/cpu-init/Makefile.inc

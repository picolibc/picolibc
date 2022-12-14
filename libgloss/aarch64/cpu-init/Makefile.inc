%C%_CPPFLAGS = -I$(srcdir)/%D%/..

%C%cpuinitdir = $(tooldir)/lib$(MULTISUBDIR)/cpu-init
%C%cpuinit_DATA = \
	%D%/rdimon-aem-el3.o \
	%D%/rdimon-aem-v8-r.o

## The manual dirstamp is needed as automake doesn't recognize compile rules
## via DATA.  If we ever add rules it does know about, we can drop this.
%D%/$(am__dirstamp):
	@$(MKDIR_P) %D%
	@: > $@

%D%/rdimon-aem-el3.$(OBJEXT): %D%/rdimon-aem-el3.S %D%/$(am__dirstamp)
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -DARM_RDI_MONITOR -o $@ -c $<

%D%/rdimon-aem-v8-r.$(OBJEXT): %D%/rdimon-aem-el3.S %D%/$(am__dirstamp)
	$(AM_V_CC)$(COMPILE) $(%C%_CPPFLAGS) -DARM_RDI_MONITOR -DBUILD_FOR_R_PROFILE -o $@ -c $<

CLEANFILES += $(%C%cpuinit_DATA)

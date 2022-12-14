%C%cpuinitdir = $(tooldir)/lib$(MULTISUBDIR)/cpu-init
%C%cpuinit_DATA = \
	%D%/rdimon-aem.o

## The manual dirstamp is needed as automake doesn't recognize compile rules
## via DATA.  If we ever add rules it does know about, we can drop this.
%D%/$(am__dirstamp):
	@$(MKDIR_P) %D%
	@: > $@

%D%/rdimon-aem.$(OBJEXT): %D%/rdimon-aem.S %D%/$(am__dirstamp)
	@$(MKDIR_P) %D%
	$(AM_V_CC)$(COMPILE) -DARM_RDI_MONITOR -o $@ -c $<

CLEANFILES += $(%C%cpuinit_DATA)

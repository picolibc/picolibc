if !MAY_SUPPLY_SYSCALLS
multilibtool_DATA += %D%/crt0.o
endif

%C%_SPECS =

## nano files.
%C%_SPECS += %D%/nano.specs

## Linux files.
%C%_SPECS += %D%/linux.specs
multilibtool_DATA += %D%/linux-crt0.o
multilibtool_LIBRARIES += %D%/libgloss-linux.a
%C%_libgloss_linux_a_SOURCES = \
	%D%/linux-syscalls0.c \
	%D%/linux-syscalls1.c

## RedBoot files.
%C%_SPECS += %D%/redboot.ld %D%/redboot.specs
multilibtool_DATA += \
	%D%/redboot-crt0.o \
	%D%/redboot-syscalls.o

%D%/redboot-syscalls.o: %D%/redboot-syscalls.c
	$(AM_V_CC)$(COMPILE) -DSEMIHOST_V2 -o $@ -c $<

## rdpmon files.
%C%_SPECS += %D%/rdpmon.specs
multilibtool_DATA += %D%/rdpmon-crt0.o
multilibtool_LIBRARIES += %D%/librdpmon.a
%C%_librdpmon_a_SOURCES = \
	%D%/_exit.c \
	%D%/_kill.c \
	%D%/ftruncate.c \
	%D%/libcfunc.c \
	%D%/syscalls.c \
	%D%/trap.S \
	%D%/truncate.c

%D%/rdpmon-crt0.o: %D%/crt0.S
	$(AM_V_CPPAS)$(CPPASCOMPILE) -DSEMIHOST_V2 -DARM_RDP_MONITOR -o $@ -c $<

## rdimon files.
%C%_SPECS += \
	%D%/rdimon.specs \
	%D%/rdimon-v2m.specs \
	%D%/aprofile-validation.specs \
	%D%/aprofile-validation-v2m.specs \
	%D%/aprofile-ve-v2m.specs \
	%D%/aprofile-ve.specs
multilibtool_DATA += \
	%D%/rdimon-crt0.o \
	%D%/rdimon-crt0-v2m.o
multilibtool_LIBRARIES += \
	%D%/librdimon.a \
	%D%/librdimon-v2m.a

%C%_RDIMON_CPPFLAGS = -DSEMIHOST_V2 -DARM_RDI_MONITOR
%C%_RDIMON_V2M_CPPFLAGS = $(%C%_RDIMON_CPPFLAGS) -DSEMIHOST_V2_MIXED_MODE

%D%/rdimon-crt0.o: %D%/crt0.S
	$(AM_V_CPPAS)$(CPPASCOMPILE) $(%C%_RDIMON_CPPFLAGS) -o $@ -c $<

%D%/rdimon-crt0-v2m.o: %D%/crt0.S
	$(AM_V_CPPAS)$(CPPASCOMPILE) $(%C%_RDIMON_V2M_CPPFLAGS) -o $@ -c $<

%C%_librdimon_a_SOURCES = $(%C%_librdpmon_a_SOURCES)
%C%_librdimon_a_CPPFLAGS = $(AM_CPPFLAGS) $(%C%_RDIMON_CPPFLAGS)
%C%_librdimon_v2m_a_SOURCES = $(%C%_librdpmon_a_SOURCES)
%C%_librdimon_v2m_a_CPPFLAGS = $(AM_CPPFLAGS) $(%C%_RDIMON_V2M_CPPFLAGS)

## Here is all of the eval board stuff.
%C%_SPECS += \
	%D%/pid.specs \
	%D%/iq80310.specs

multilibtool_DATA += \
	$(%C%_SPECS)

## Make copies of the scripts to facilitate in-tree bootstrapping.
%D%/%.specs: %D%/$(ARM_OBJTYPE)%.specs
	$(AM_V_GEN)cp $< $@

%D%/%.ld: %D%/$(ARM_OBJTYPE)%.ld
	$(AM_V_GEN)cp $< $@

CLEANFILES += $(%C%_SPECS)

include %D%/cpu-init/Makefile.inc

if MCORE_BUILD_ELF
multilibtool_DATA += \
	%D%/cmb.ld \
	%D%/cmb.specs
%D%/cmb.ld: %D%/elf-cmb.ld
	$(AM_V_GEN)cp $< $@
%D%/cmb.specs: %D%/elf-cmb.specs
	$(AM_V_GEN)cp $< $@
endif
if MCORE_BUILD_PE
multilibtool_DATA += \
	%D%/cmb.ld \
	%D%/cmb.specs
%D%/cmb.ld: %D%/pe-cmb.ld
	$(AM_V_GEN)cp $< $@
%D%/cmb.specs: %D%/pe-cmb.specs
	$(AM_V_GEN)cp $< $@
endif

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

%C%_common_lib_sources = \
	%D%/fstat.c \
	%D%/getpid.c \
	isatty.c \
	%D%/kill.c \
	%D%/raise.c \
	%D%/putnum.c \
	%D%/stat.c \
	%D%/unlink.c

## Here is all of the simulator stuff
multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/syscalls.S \
	$(%C%_common_lib_sources)

## Here is all of the picobug on cmb stuff
multilibtool_LIBRARIES += %D%/libcmb.a
%C%_libcmb_a_SOURCES = \
	%D%/open.c \
	%D%/close.c \
	%D%/lseek.c \
	%D%/sbrk.c \
	%D%/read.c \
	%D%/write.c \
	%D%/print.c \
	%D%/cmb-exit.c \
	%D%/cmb-inbyte.c \
	%D%/cmb-outbyte.c \
	$(%C%_common_lib_sources)

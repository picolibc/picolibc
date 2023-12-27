multilibtool_DATA += \
	%D%/crt0.o \
	%D%/crti.o \
	%D%/crtn.o
libobjs_a_SOURCES += \
	%D%/crt0.s \
	%D%/crti.s \
	%D%/crtn.s

## Here is all of the simulator stuff
multilibtool_DATA += \
	%D%/sim_high.ld \
	%D%/sim_rom.ld
multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/syscalls.S \
	%D%/sim_malloc_start.s \
	%D%/isatty.c

## Here is stuff for building apps for GDB on the EVA board
multilibtool_DATA += %D%/eva_app.ld
multilibtool_LIBRARIES += %D%/libeva_app.a
%C%_libeva_app_a_SOURCES = \
	%D%/close.c \
	%D%/fstat.c \
	%D%/getpid.c \
	%D%/kill.c \
	%D%/lseek.c \
	%D%/open.c \
	%D%/stat.c \
	%D%/unlink.c \
	%D%/eva_app.c \
	%D%/isatty.c

## Stub stuff
multilibtool_PROGRAMS += %D%/eva_stub.elf
%C%_eva_stub_elf_SOURCES = \
	%D%/xstormy16_stub.c \
	%D%/crt0_stub.s
%C%_eva_stub_elf_LDFLAGS = \
	$(AM_LDFLAGS) \
	-nostartfiles -T$(srcdir)/%D%/eva_stub.ld

multilibtool_DATA += \
	%D%/eva_stub.hex
%D%/eva_stub.hex: %D%/eva_stub.elf
	$(AM_V_GEN)$(OBJCOPY) -O ihex $< $@
CLEANFILES += %D%/eva_stub.hex

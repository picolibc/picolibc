## Copyright (c) 1995, 1996, 1997 Cygnus Support
##
## The authors hereby grant permission to use, copy, modify, distribute,
## and license this software and its documentation for any purpose, provided
## that existing copyright notices are retained in all copies and that this
## notice is included verbatim in any distributions. No written agreement,
## license, or royalty fee is required for any of the authorized uses.
## Modifications to this software may be copyrighted by their authors
## and need not follow the licensing terms described here, provided that
## the new terms are clearly indicated on the first page of each file where
## they apply.

## This is set to one of SPARC, SLITE, or SPLET by configure.
## It's not clear what to do here.
## One could certainly build everything.  The assembler supports all cpu
## variants (via runtime switches).  However, the compiler [currently] doesn't.
## Of course, it may be the case that there isn't any cpu specific code in
## C source files, but there might be in the future.
AM_CPPFLAGS_%C% = -DTARGET_CPU_$(SPARC_CPU)

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

## Used by some libs.
%C%_common_stub_sources = \
	close.c \
	fstat.c \
	getpid.c \
	isatty.c \
	kill.c \
	lseek.c \
	open.c \
	print.c \
	putnum.c \
	read.c \
	sbrk.c \
	stat.c \
	unlink.c \
	write.c

## ERC32: SIS simulator, see sim/erc32.
multilibtool_DATA += %D%/erc32-crt0.o
libobjs_a_SOURCES += %D%/erc32-crt0.S
multilibtool_LIBRARIES += %D%/liberc32.a
%C%_liberc32_a_SOURCES = \
	$(%C%_common_stub_sources) \
	%D%/erc32-io.c \
	%D%/traps.S \
	%D%/erc32-stub.c \
	debug.c \
	%D%/fixctors.c

if SPARC_BUILD_CYGMON
## sparc stuff (not sparclite or sparclet).
%C%_SPARC_OBJ_FORMAT = sparc
%C%_SPARC_RAM_START = 0x4000

## sparc 64 stuff.
%C%_SPARC64_OBJ_FORMAT = sparc:v9
%C%_SPARC64_RAM_START = 0x4000

## sparclite stuff.
%C%_SLITE_OBJ_FORMAT = sparc
%C%_SLITE_RAM_START = 0x40050000

multilibtool_DATA += %D%/cygmon.ld
%D%/cygmon.ld: $(srcdir)/%D%/@SPARC_CYGMONLDSCRIPTTEMPL@ %D%/Makefile.inc
	$(AM_V_GEN)sed 's/TARGET_OBJ_FORMAT/$(%C%_$(SPARC_CPU)_OBJ_FORMAT)/g;s/TARGET_RAM_START/$(%C%_$(SPARC_CPU)_RAM_START)/g;' < $< > $@

multilibtool_DATA += %D%/cygmon-crt0.o
libobjs_a_SOURCES += %D%/cygmon-crt0.S
multilibtool_LIBRARIES += %D%/libcygmon.a
%C%_libcygmon_a_SOURCES = \
	%D%/cygmon-salib.c
multilibtool_DATA += %D%/cygmon-salib.o
endif

if SPARC_BUILD_SLITE
multilibtool_DATA += \
	%D%/ex930.ld \
	%D%/ex931.ld \
	%D%/ex934.ld \
	%D%/sparc86x.ld \
	%D%/elfsim.ld \
	%D%/traps.o

## For the time being, built the stub without hardware breakpoint support.
multilibtool_LIBRARIES += \
	%D%/libslite930.a \
	%D%/libslite931.a \
	%D%/libslite934.a \
	%D%/libslite86x.a
%C%_libslite930_a_SOURCES = \
	$(%C%_common_stub_sources) \
	%D%/salib.c \
	%D%/sparcl-stub.c \
	%D%/cache.c
%C%_libslite931_a_SOURCES = $(%C%_libslite930_a_SOURCES)
%C%_libslite934_a_SOURCES = $(%C%_libslite930_a_SOURCES)
%C%_libslite86x_a_SOURCES = $(%C%_libslite930_a_SOURCES)
endif

if SPARC_BUILD_SPLET
multilibtool_DATA += %D%/tsc701.ld

multilibtool_DATA += %D%/crt0-701.S
libobjs_a_SOURCES += %D%/crt0-701.S

multilibtool_LIBRARIES += %D%/libsplet701.a
%C%_libsplet701_a_SOURCES = \
	%D%/sysc-701.c \
	%D%/salib-701.c \
	%D%/sparclet-stub.c
endif

## Compile a fully linked binary. The -N option is for a.out, so the
## base address will be zero, rather than the default of 0x2020. The
## -Wl,-T*.ld is for the linker script. By using -Wl, the linker script
## is put on the proper place in the comand line for ld, and all the
## symbols will get fully resolved.

check_PROGRAMS += %D%/erc32-test
%C%_erc32_test_SOURCES = %D%/test.c
AM_LDFLAGS_%C%_erc32_test = -N -Wl,-T$(srcdir)/%D%/elfsim.ld -Wl,-Map,$@.map -nostdlib

check_PROGRAMS += %D%/ex930-test
%C%_ex930_test_SOURCES = %D%/test.c
AM_LDFLAGS_%C%_ex930_test = -N -Wl,-T$(srcdir)/%D%/ex930.ld -nostdlib

check_PROGRAMS += %D%/ex931-test
%C%_ex931_test_SOURCES = %D%/test.c
AM_LDFLAGS_%C%_ex931_test = -N -Wl,-T$(srcdir)/%D%/ex931.ld -nostdlib

check_PROGRAMS += %D%/ex934-test
%C%_ex934_test_SOURCES = %D%/test.c
AM_LDFLAGS_%C%_ex934_test = -N -Wl,-T$(srcdir)/%D%/ex934.ld -nostdlib

## A C++ test case.
# check_PROGRAMS += %D%/dtor
# %C%_ex934_test_SOURCES = %D%/dtor.C
# %C%_ex934_test_LDFLAGS = $(AM_LDFLAGS) -N -Wl,-T$(srcdir)/%D%/elfsim.ld -nostdlib

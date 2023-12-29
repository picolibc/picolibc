## Copyright (C) 2015 FTDI (support@ftdichip.com)
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

if !HAVE_MULTISUBDIR
multilibtool_DATA += \
	%D%/sim.ld
endif

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/fstat.c \
	%D%/getpid.c \
	%D%/isatty.c \
	%D%/kill.c \
	%D%/sbrk.c \
	%D%/sim-close.S \
	%D%/sim-exit.S \
	%D%/sim-inbyte.c \
	%D%/sim-lseek.c \
	%D%/sim-open.S \
	%D%/sim-read.S \
	%D%/sim-time.c \
	%D%/sim-unlink.S \
	%D%/sim-write.S \
	%D%/stat.c

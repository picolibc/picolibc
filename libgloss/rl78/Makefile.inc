## Copyright (c) 2008, 2009, 2011 Red Hat Inc
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

multilibtool_DATA += \
	%D%/rl78.ld \
	%D%/rl78-sim.ld

multilibtool_DATA += \
	%D%/gcrt0.o \
	%D%/crt0.o \
	%D%/crtn.o
libobjs_a_SOURCES += \
	%D%/gcrt0.S \
	%D%/crt0.S \
	%D%/crtn.S

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/sbrk.c \
	%D%/exit.S \
	%D%/open.S \
	%D%/close.S \
	%D%/read.S \
	%D%/write.c \
	%D%/swrite.S \
	%D%/lseek.S \
	%D%/unlink.S \
	%D%/getpid.S \
	%D%/kill.S \
	%D%/fstat.S \
	%D%/argvlen.S \
	%D%/argv.S \
	%D%/chdir.S \
	%D%/stat.S \
	%D%/chmod.S \
	%D%/utime.S \
	%D%/time.S \
	%D%/gettimeofday.S \
	%D%/times.S \
	%D%/link.S \
	%D%/isatty.S \
	%D%/abort.S \
	%D%/mcount.c

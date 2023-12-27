## Copyright (c) 1998 Cygnus Support
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
	%D%/eval.ld \
	%D%/sim.ld

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

multilibtool_LIBRARIES += %D%/libeval.a
%C%_libeval_a_SOURCES = \
	%D%/_exit.c \
	%D%/access.c \
	%D%/chmod.c \
	%D%/close.c \
	%D%/crt1.c \
	%D%/fstat.c \
	%D%/getpid.c \
	%D%/isatty.c \
	%D%/kill.c \
	%D%/lseek.c \
	%D%/open.c \
	%D%/read.c \
	%D%/sbrk.c \
	%D%/stat.c \
	%D%/time.c \
	%D%/trap.S \
	%D%/unlink.c \
	%D%/utime.c \
	%D%/write.c

check_PROGRAMS += %D%/test
%C%_test_LDFLAGS = $(AM_LDFLAGS) -Wl,-T$(srcdir)/%D%/eval.ld

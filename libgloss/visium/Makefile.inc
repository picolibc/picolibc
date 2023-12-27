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

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

## These go with the hosted file I/O libraries with a version of syscalls.
%C%_hosted_sources = \
	getpid.c \
	kill.c \
	%D%/io-gdb.c \
	%D%/io-stubs.c \
	%D%/sbrk.c

multilibtool_LIBRARIES += %D%/libserial.a
%C%_libserial_a_SOURCES = \
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
	stat.c \
	unlink.c \
	write.c \
	%D%/_exit.c \
	%D%/gettod.c \
	%D%/sbrk.c \
	%D%/serial-inbyte.c \
	%D%/serial-outbyte.c

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	$(%C%_hosted_sources) \
	%D%/sim-syscalls.c

multilibtool_LIBRARIES += %D%/libdebug.a
%C%_libdebug_a_SOURCES = \
	$(%C%_hosted_sources) \
	%D%/syscalls.c

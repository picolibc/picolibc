## Makefile for libgloss/nds32
## Copyright (c) 1996, 1998 Cygnus Support. 2012 Andes Porting.
## All rights reserved.
##
## Redistribution and use in source and binary forms are permitted
## provided that the above copyright notice and this paragraph are
## duplicated in all such forms and that any documentation,
## and/or other materials related to such
## distribution and use acknowledge that the software was developed
## at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
## endorse or promote products derived from this software without
## specific prior written permission.
## THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
## IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/crt1.o
libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/crt1.S

multilibtool_LIBRARIES += %D%/libgloss.a
%C%_libgloss_a_SOURCES = \
	%D%/_exit.S \
	%D%/_open.S \
	%D%/_close.S \
	%D%/_read.S \
	%D%/_write.S \
	%D%/_lseek.S \
	%D%/_unlink.S \
	%D%/_getpid.S \
	%D%/_kill.S \
	%D%/_fstat.S \
	%D%/_argvlen.S \
	%D%/_argv.S \
	%D%/_chdir.S \
	%D%/_stat.S \
	%D%/_chmod.S \
	%D%/_utime.S \
	%D%/_time.S \
	%D%/_gettimeofday.S \
	%D%/_times.S \
	%D%/_link.S \
	%D%/_rename.S \
	%D%/_isatty.S \
	%D%/_system.S \
	%D%/_sbrk.S \
	%D%/syscall_error_handler.S

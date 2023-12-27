## Makefile for libgloss/m32r
## Copyright (c) 1996, 1998 Cygnus Support.
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

if !HAVE_MULTISUBDIR
multilibtool_DATA += \
	%D%/eva.ld \
	%D%/eva-stub.ld \
	%D%/mon.specs
endif

multilibtool_DATA += %D%/crt0.o
libobjs_a_SOURCES += %D%/crt0.S

%C%_common_lib_sources = \
	%D%/chmod.c \
	%D%/close.c \
	%D%/exit.c \
	%D%/fstat.c \
	%D%/getpid.c \
	%D%/isatty.c \
	%D%/kill.c \
	%D%/lseek.c \
	%D%/open.c \
	%D%/raise.c \
	%D%/read.c \
	%D%/sbrk.c \
	%D%/stat.c \
	%D%/unlink.c \
	%D%/utime.c \
	%D%/write.c

multilibtool_LIBRARIES += %D%/libgloss.a
%C%_libgloss_a_SOURCES = \
	%D%/trap0.S \
	$(%C%_common_lib_sources)

multilibtool_LIBRARIES += %D%/libmon.a
%C%_libmon_a_SOURCES = \
	%D%/trapmon0.c \
	$(%C%_common_lib_sources)

multilibtool_DATA += \
	%D%/m32r-lib.o \
	%D%/m32r-stub.o
libobjs_a_SOURCES += \
	%D%/m32r-lib.c \
	%D%/m32r-stub.c

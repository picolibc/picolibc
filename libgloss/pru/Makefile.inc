## SPDX-License-Identifier: BSD-2-Clause-FreeBSD
##
## Copyright 2018-2024 Dimitar Dimitrov <dimitar@dinux.eu>
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
## IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
## OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
## IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
## INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
## NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
## DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
## THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
## THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/crt0-minrt.o

libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/crt0-minrt.S

multilibtool_LIBRARIES += %D%/libgloss.a
%C%_libgloss_a_SOURCES = \
	%D%/do_global_dtors.c \
	%D%/gettimeofday.c \
	%D%/isatty.c \
	%D%/putnum.c \
	%D%/raise.c \
	%D%/inbyte.c \
	%D%/outbyte.c \
	%D%/sbrk.c \
	%D%/stat.c \
	%D%/syscalls.S

pru_devicespecsdir = $(multilibtooldir)/device-specs
pru_devicespecs_DATA = %D%/device-specs/sim

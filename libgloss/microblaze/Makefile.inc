##  Copyright 2007, 2009 Xilinx, Inc.  All rights reserved.
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions are
##  met:
##
##  1.  Redistributions source code must retain the above copyright notice,
##  this list of conditions and the following disclaimer.
##
##  2.  Redistributions in binary form must reproduce the above copyright
##  notice, this list of conditions and the following disclaimer in the
##  documentation and/or other materials provided with the distribution.
##
##  3.  Neither the name of Xilinx nor the names of its contributors may be
##  used to endorse or promote products derived from this software without
##  specific prior written permission.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
##  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
##  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
##  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
##  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
##  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
##  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
##  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
##  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
##  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
##  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if !HAVE_MULTISUBDIR
multilibtool_DATA += \
	%D%/xilinx.ld
endif
multilibtool_DATA += \
	%D%/elf-gloss-linux.specs

multilibtool_DATA += \
	%D%/crt0.o \
	%D%/crt1.o \
	%D%/crt2.o \
	%D%/crt3.o \
	%D%/crt4.o \
	%D%/crtinit.o \
	%D%/pgcrtinit.o \
	%D%/sim-crtinit.o \
	%D%/sim-pgcrtinit.o \
	%D%/linux-crt0.o
libobjs_a_SOURCES += \
	%D%/crt0.S \
	%D%/crt1.S \
	%D%/crt2.S \
	%D%/crt3.S \
	%D%/crt4.S \
	%D%/crtinit.S \
	%D%/pgcrtinit.S \
	%D%/sim-crtinit.S \
	%D%/sim-pgcrtinit.S \
	%D%/linux-crt0.S

multilibtool_LIBRARIES += %D%/libgloss.a
%C%_libgloss_a_SOURCES = \
	fstat.c \
	getpid.c \
	isatty.c \
	kill.c \
	lseek.c \
	print.c \
	putnum.c \
	stat.c \
	unlink.c \
	open.c \
	close.c \
	read.c \
	write.c \
	%D%/sbrk.c \
	%D%/timer.c \
	%D%/_exception_handler.S \
	%D%/_hw_exception_handler.S \
	%D%/_interrupt_handler.S \
	%D%/_program_clean.S \
	%D%/_program_init.S \
	%D%/xil_malloc.c \
	%D%/xil_sbrk.c \
	%D%/xil_printf.c

multilibtool_LIBRARIES += %D%/libgloss-linux.a
%C%_libgloss_linux_a_SOURCES = \
	%D%/linux-syscalls.S \
	%D%/linux-syscalls-wrap.c \
	%D%/linux-outbyte.c \
	%D%/linux-inbyte.c

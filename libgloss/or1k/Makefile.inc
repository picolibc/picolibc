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

includetool_DATA += \
	%D%/include/or1k-asm.h \
	%D%/include/or1k-nop.h \
	%D%/include/or1k-sprs.h \
	%D%/include/or1k-support.h

multilibtool_LIBRARIES += %D%/libor1k.a
%C%_libor1k_a_SOURCES = \
	%D%/syscalls.c \
	%D%/or1k_uart.c \
	%D%/outbyte.c \
	%D%/caches-asm.S \
	%D%/exceptions.c \
	%D%/exceptions-asm.S \
	%D%/interrupts.c \
	%D%/interrupts-asm.S \
	%D%/mmu-asm.S \
	%D%/timer.c \
	%D%/sbrk.c \
	%D%/impure.c \
	%D%/util.c \
	%D%/sync-asm.S

## Now all the board definitions.

multilibtool_LIBRARIES += %D%/libboard-atlys.a
%C%_libboard_atlys_a_SOURCES = %D%/boards/atlys.S

multilibtool_LIBRARIES += %D%/libboard-de0_nano.a
%C%_libboard_de0_nano_a_SOURCES = %D%/boards/de0_nano.S

multilibtool_LIBRARIES += %D%/libboard-ml501.a
%C%_libboard_ml501_a_SOURCES = %D%/boards/ml501.S

multilibtool_LIBRARIES += %D%/libboard-optimsoc.a
%C%_libboard_optimsoc_a_SOURCES = %D%/boards/optimsoc.S

multilibtool_LIBRARIES += %D%/libboard-or1ksim.a
%C%_libboard_or1ksim_a_SOURCES = %D%/boards/or1ksim.S

multilibtool_LIBRARIES += %D%/libboard-or1ksim-uart.a
%C%_libboard_or1ksim_uart_a_SOURCES = %D%/boards/or1ksim-uart.S

multilibtool_LIBRARIES += %D%/libboard-ordb1a3pe1500.a
%C%_libboard_ordb1a3pe1500_a_SOURCES = %D%/boards/ordb1a3pe1500.S

multilibtool_LIBRARIES += %D%/libboard-ordb2a.a
%C%_libboard_ordb2a_a_SOURCES = %D%/boards/ordb2a.S

multilibtool_LIBRARIES += %D%/libboard-orpsocrefdesign.a
%C%_libboard_orpsocrefdesign_a_SOURCES = %D%/boards/orpsocrefdesign.S

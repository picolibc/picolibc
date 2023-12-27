## Copyright (c) 2008-2014 Red Hat, Inc. All rights reserved.
##
## This copyrighted material is made available to anyone wishing to use, modify,
## copy, or redistribute it subject to the terms and conditions of the BSD
## License.   This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
## of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
## is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
## are incorporated in the source code or documentation are not subject to the BSD
## License and may only be used or replicated with the express permission of
## Red Hat, Inc.

multilibtool_DATA += \
	%D%/msp430-sim.ld \
	%D%/msp430xl-sim.ld \
	%D%/intr_vectors.ld

multilibtool_DATA += \
	%D%/gcrt0.o \
	%D%/crt0.o
libobjs_a_SOURCES += \
	%D%/gcrt0.S \
	%D%/crt0.S

multilibtool_LIBRARIES += %D%/libsim.a
%C%_libsim_a_SOURCES = \
	%D%/syscalls.S \
	%D%/cio.S \
	%D%/write.c \
	%D%/unlink.c \
	%D%/sbrk.c

multilibtool_LIBRARIES += %D%/libnosys.a
%C%_libnosys_a_SOURCES = \
	%D%/ciosyscalls.S \
	%D%/cio.S \
	%D%/write.c \
	%D%/unlink.c \
	%D%/sbrk.c

multilibtool_LIBRARIES += %D%/libcrt.a
%C%_libcrt_a_SOURCES = \
	%D%/crt_bss.S \
	%D%/crt_high_bss.S \
	%D%/crt_movedata.S \
	%D%/crt_move_highdata.S \
	%D%/crt_main.S \
	%D%/crt_callexit.S \
	%D%/crt_run_init_array.S \
	%D%/crt_run_preinit_array.S \
	%D%/crt_run_fini_array.S \
	%D%/crt_run_array.S

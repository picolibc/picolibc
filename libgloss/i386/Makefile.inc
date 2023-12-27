## Copyright (c) 1997, 2000 Cygnus Support
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
	%D%/cygmon.ld

multilibtool_DATA += %D%/cygmon-crt0.o
libobjs_a_SOURCES += %D%/cygmon-crt0.S

multilibtool_LIBRARIES += %D%/libcygmon.a
%C%_libcygmon_a_SOURCES = \
	%D%/cygmon-salib.c \
	%D%/cygmon-gmon.c
%C%_libcygmon_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(I386_CPPFLAGS)

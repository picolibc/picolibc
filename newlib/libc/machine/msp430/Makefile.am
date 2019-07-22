# Copyright (c) 2013 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the BSD License.   This program is distributed in the hope that
# it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
# including the implied warranties of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  A copy of this license is available at
# http://www.opensource.org/licenses. Any Red Hat trademarks that are
# incorporated in the source code or documentation are not subject to
# the BSD License and may only be used or replicated with the express
# permission of Red Hat, Inc.

## Process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)

AM_CCASFLAGS = $(INCLUDES)

noinst_LIBRARIES = lib.a

lib_a_SOURCES = setjmp.S $(TINY_SOURCES)
lib_a_CCASFLAGS=$(AM_CCASFLAGS)
lib_a_CFLAGS=$(AM_CFLAGS)

# tiny-printf.c and tiny-puts.c are derived from the nano printf/puts
# functions, so other supporting nano functions are required, and the tiny
# printf/puts will not work without them.
if NEWLIB_NANO_FORMATTED_IO
TINY_SOURCES = tiny-puts.c tiny-printf.c
endif

ACLOCAL_AMFLAGS = -I ../../.. -I ../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host

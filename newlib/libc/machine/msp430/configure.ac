# Copyright (c) 2013  Red Hat, Inc. All rights reserved.
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

dnl This is the newlib/libc/machine/msp430 configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([setjmp.S])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../..)

dnl Support --enable-newlib-nano-formatted-io required by tiny-printf.c
dnl and tiny-puts.c
AC_ARG_ENABLE(newlib_nano_formatted_io,
[  --enable-newlib-nano-formatted-io    Use small-footprint nano-formatted-IO implementation],
[case "${enableval}" in
   yes) newlib_nano_formatted_io=yes ;;
   no)  newlib_nano_formatted_io=no ;;
   *) AC_MSG_ERROR(bad value ${enableval} for newlib-nano-formatted-io) ;;
 esac],[newlib_nano_formatted_io=no])
AM_CONDITIONAL(NEWLIB_NANO_FORMATTED_IO, test x$newlib_nano_formatted_io = xyes)

NEWLIB_CONFIGURE(../../..)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl Copyright (c) 2006, Red Hat, Inc.
dnl
dnl     This program is free software; you can redistribute it and/or modify
dnl     it under the terms of the GNU General Public License as published by
dnl     the Free Software Foundation; either version 2 of the License, or
dnl     (at your option) any later version.
dnl
dnl     A copy of the GNU General Public License can be found at
dnl     http://www.gnu.org/
dnl
dnl Written by Christopher Faylor <cgf@redhat.com>
dnl Changed for lsa subdir by Corinna Vinschen <corinna@vinschen.de>

dnl Autoconf configure script for Cygwin utilities.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Makefile.in)

AC_CONFIG_AUX_DIR(../..)

AC_NO_EXECUTABLES
AC_CANONICAL_SYSTEM

LIB_AC_PROG_CC

case "$target_cpu" in
i?86)
  AC_CHECK_PROGS(MINGW32_CC, i686-w64-mingw32-gcc)
  test -z "$MINGW32_CC" && AC_MSG_ERROR([no acceptable mingw32 cc found in \$PATH])
  ;;
esac

AC_CHECK_PROGS(MINGW64_CC, x86_64-w64-mingw32-gcc)
test -z "$MINGW64_CC" && AC_MSG_ERROR([no acceptable mingw64 cc found in \$PATH])

AC_ARG_PROGRAM

AC_PROG_INSTALL

AC_OUTPUT(Makefile cyglsa.def:cyglsa.din)

dnl Autoconf configure script for Cygwin utilities.
dnl
dnl This file is part of Cygwin.
dnl 
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.
dnl
dnl Written by Christopher Faylor <cgf@redhat.com>
dnl Changed for lsa subdir by Corinna Vinschen <corinna@vinschen.de>

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([Cygwin Lsaauth],[0],[cygwin@cygwin.com],[cygwin],[https://cygwin.com])
AC_CONFIG_SRCDIR(Makefile.in)

AC_CONFIG_AUX_DIR(../..)

AC_NO_EXECUTABLES
AC_CANONICAL_TARGET

AC_PROG_CC

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

AC_CONFIG_FILES([Makefile cyglsa.def:cyglsa.din])
AC_OUTPUT

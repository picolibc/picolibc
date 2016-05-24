dnl Autoconf configure script for Cygwin utilities.
dnl
dnl This file is part of Cygwin.
dnl
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([Cygwin Utils],[0],[cygwin@cygwin.com],[cygwin],[https://cygwin.com])
AC_CONFIG_SRCDIR(mount.cc)
AC_CONFIG_AUX_DIR(../..)

AC_NO_EXECUTABLES

. ${srcdir}/../configure.cygwin

AC_CANONICAL_TARGET

AC_PROG_CC

AC_WINDOWS_HEADERS
AC_WINDOWS_LIBS

AC_PROG_CXX

AC_CYGWIN_INCLUDES

AC_ARG_PROGRAM

AC_PROG_INSTALL

AC_CHECK_PROGS(MINGW_CXX, ${target_cpu}-w64-mingw32-g++)
test -n "$MINGW_CXX" || AC_MSG_ERROR([no acceptable mingw g++ found in \$PATH])

AC_EXEEXT
AC_CONFIGURE_ARGS
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

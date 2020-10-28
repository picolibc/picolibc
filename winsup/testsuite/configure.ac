dnl Autoconf configure script for Cygwin's testsuite.
dnl
dnl This file is part of Cygwin.
dnl
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([Cygwin Testsuite],[0],[cygwin@cygwin.com],[cygwin],[https://cygwin.com])
AC_CONFIG_SRCDIR(Makefile.in)

. ${srcdir}/../configure.cygwin

AC_CANONICAL_TARGET

AC_PROG_CC
AC_PROG_CPP
AC_CHECK_TOOL(AR,ar)

AC_SUBST(target_builddir)

AC_CHECK_PROGS(MINGW_CXX, ${target_cpu}-w64-mingw32-g++)
AC_CHECK_PROGS(MINGW_CC, ${target_cpu}-w64-mingw32-gcc)

AC_OUTPUT(Makefile)

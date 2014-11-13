dnl Autoconf configure script for Cygwin.
dnl Copyright 2003 Red Hat, Inc.
dnl
dnl This file is part of Cygwin.
dnl
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)dnl
AC_INIT(cygserver.cc)
AC_CONFIG_AUX_DIR(..)

. ${srcdir}/../configure.cygwin

AC_PROG_INSTALL
AC_NO_EXECUTABLES
AC_CANONICAL_SYSTEM

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_LANG(C)

AC_WINDOWS_HEADERS
AC_WINDOWS_LIBS

AC_LANG(C++)

AC_CYGWIN_INCLUDES

case "$with_cross_host" in
  ""|*cygwin*)
    all_host="all_host"
    install_host="install_host"
    ;;
  *)
    all_host=
    install_host=
    ;;
esac

AC_SUBST(all_host)
AC_SUBST(install_host)

AC_CHECK_TOOL(AR, ar, ar)
AC_CHECK_TOOL(AS, as, as)
AC_CHECK_TOOL(RANLIB, ranlib, ranlib)
AC_CHECK_TOOL(LD, ld, ld)
AC_CHECK_TOOL(NM, nm, nm)
AC_CHECK_TOOL(DLLTOOL, dlltool, dlltool)
AC_CHECK_TOOL(WINDRES, windres, windres)

AC_PROG_MAKE_SET

AC_ARG_ENABLE(debugging,
[ --enable-debugging		Build a cygwin DLL which has more consistency checking for debugging],
[case "${enableval}" in
yes)	 AC_DEFINE(DEBUGGING) ;;
no)	 ;;
esac
])

AC_CONFIGURE_ARGS
AC_OUTPUT(Makefile)

dnl Autoconf configure script for Cygwin.
dnl
dnl This file is part of Cygwin.
dnl
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([Cygwin toplevel],[0],[cygwin@cygwin.com],[cygwin],[https://cygwin.com])
AC_CONFIG_SRCDIR(Makefile.in)
AC_CONFIG_AUX_DIR(..)

. ${srcdir}/configure.cygwin

AC_PROG_INSTALL
AC_NO_EXECUTABLES
AC_CANONICAL_TARGET

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_LANG(C)
AC_ARG_WITH([cross-bootstrap],[AS_HELP_STRING([--with-cross-bootstrap],[do not build programs using the mingw toolchain or check for mingw libraries (useful for bootstrapping a cross-compiler)])],[],[with_cross_bootstrap=no])

AC_WINDOWS_HEADERS
if test "x$with_cross_bootstrap" != "xyes"; then
    AC_WINDOWS_LIBS
fi

AC_LANG(C++)

AC_CYGWIN_INCLUDES

AC_CONFIG_SUBDIRS(cygwin cygserver doc)
if test "x$with_cross_bootstrap" != "xyes"; then
    AC_CONFIG_SUBDIRS([utils])
fi

INSTALL_LICENSE="install-license"

AC_SUBST(INSTALL_LICENSE)

AC_PROG_MAKE_SET

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

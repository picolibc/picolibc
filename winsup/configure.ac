dnl Autoconf configure script for Cygwin.
dnl Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2006, 2009, 2010,
dnl 2011, 2012 Red Hat, Inc.
dnl
dnl This file is part of Cygwin.
dnl
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)dnl
AC_INIT(Makefile.in)
AC_CONFIG_AUX_DIR(..)

. ${srcdir}/configure.cygwin

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

AC_CONFIG_SUBDIRS(cygwin utils cygserver lsaauth doc)
INSTALL_LICENSE="install-license"

AC_SUBST(INSTALL_LICENSE)

AC_PROG_MAKE_SET

AC_OUTPUT(Makefile)

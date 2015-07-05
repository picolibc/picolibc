dnl Autoconf configure script for winsup/doc
dnl Copyright 1997, 1998, 1999, 2000, 2001, 2006, 2012, 2013, 2014 Red Hat, Inc.
dnl
dnl This file is part of Cygwin.
dnl 
dnl This software is a copyrighted work licensed under the terms of the
dnl Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
dnl details.

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([Cygwin Documentation], 0,
	cygwin@cygwin.com, cygwin, https://cygwin.com)
AC_CONFIG_SRCDIR(cygwin-api.xml)
AC_CONFIG_AUX_DIR(../..)

AC_PROG_INSTALL
AC_CHECK_PROGS([DOCBOOK2XTEXI], [docbook2x-texi db2x_docbook2texi], [true])
AC_NO_EXECUTABLES
AC_CANONICAL_SYSTEM

LIB_AC_PROG_CC

AC_SUBST(build_exeext)

AC_OUTPUT(Makefile)

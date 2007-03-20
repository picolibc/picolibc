# aclocal.m4 for MinGW Runtime package.  -*- Autoconf -*-
#
# This provides configure definitions used by all the winsup
# configure.in files.

# MINGW_AC_CONFIG_SRCDIR( VERSION_TAG, UNIQUE_FILE )
# --------------------------------------------------
# Wrapper for AC_CONFIG_SRCDIR; in addition to checking for a
# unique file reference within the source tree, it resolves the
# definition for PACKAGE_VERSION, based on a tagged definition
# within that file, and adjusts PACKAGE_TARNAME to match.
#
AC_DEFUN([MINGW_AC_CONFIG_SRCDIR],
[AC_CONFIG_SRCDIR([$2])
 AC_MSG_CHECKING([package version])
 PACKAGE_VERSION=`awk '$[2] == "'"$1"'" { print $[3] }' ${srcdir}/$2`
 PACKAGE_TARNAME=${PACKAGE_NAME}-${PACKAGE_VERSION}
 AC_MSG_RESULT([$PACKAGE_VERSION])dnl
]) #MINGW_AC_CONFIG_SRCDIR

# The following is copied from `no-executables.m4', in the top
# `src/config' directory.
#
# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005  Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# GCC_NO_EXECUTABLES
# -----------------
# FIXME: The GCC team has specific needs which the current Autoconf
# framework cannot solve elegantly.  This macro implements a dirty
# hack until Autoconf is able to provide the services its users
# need.
#
# Several of the support libraries that are often built with GCC can't
# assume the tool-chain is already capable of linking a program: the
# compiler often expects to be able to link with some of such
# libraries.
#
# In several of these libraries, workarounds have been introduced to
# avoid the AC_PROG_CC_WORKS test, that would just abort their
# configuration.  The introduction of AC_EXEEXT, enabled either by
# libtool or by CVS autoconf, have just made matters worse.
#
# Unlike the previous AC_NO_EXECUTABLES, this test does not
# disable link tests at autoconf time, but at configure time.
# This allows AC_NO_EXECUTABLES to be invoked conditionally.
AC_DEFUN_ONCE([GCC_NO_EXECUTABLES],
[m4_divert_push([KILL])

AC_BEFORE([$0], [_AC_COMPILER_EXEEXT])
AC_BEFORE([$0], [AC_LINK_IFELSE])

m4_define([_AC_COMPILER_EXEEXT],
AC_LANG_CONFTEST([AC_LANG_PROGRAM()])
# FIXME: Cleanup?
AS_IF([AC_TRY_EVAL(ac_link)], [gcc_no_link=no], [gcc_no_link=yes])
if test x$gcc_no_link = xyes; then
  # Setting cross_compile will disable run tests; it will
  # also disable AC_CHECK_FILE but that's generally
  # correct if we can't link.
  cross_compiling=yes
  EXEEXT=
else
  m4_defn([_AC_COMPILER_EXEEXT])dnl
fi
)

m4_define([AC_LINK_IFELSE],
if test x$gcc_no_link = xyes; then
  AC_MSG_ERROR([Link tests are not allowed after [[$0]].])
fi
m4_defn([AC_LINK_IFELSE]))

dnl This is a shame.  We have to provide a default for some link tests,
dnl similar to the default for run tests.
m4_define([AC_FUNC_MMAP],
if test x$gcc_no_link = xyes; then
  if test "x${ac_cv_func_mmap_fixed_mapped+set}" != xset; then
    ac_cv_func_mmap_fixed_mapped=no
  fi
fi
if test "x${ac_cv_func_mmap_fixed_mapped}" != xno; then
  m4_defn([AC_FUNC_MMAP])
fi)

m4_divert_pop()dnl
])# GCC_NO_EXECUTABLES


# MINGW_AC_MANPAGE_TRANSFORM
# --------------------------
# Provide support for specifying a manpage name transform.
# This allows e.g. Cygwin to add a `mingw-' prefix to MinGW specific
# manpages, when installing as a Cygwin subsystem.
#
# Activated by `--enable-mingw-manpage-transform[=SED-SCRIPT]', the
# default is disabled, (i.e. no transform).  If enabled, without any
# SED-SCRIPT specification, the default `mingw-' prefix is added.
#
AC_DEFUN([MINGW_AC_MANPAGE_TRANSFORM],
[AC_ARG_ENABLE([mingw-manpage-transform],
[AS_HELP_STRING([--enable-mingw-manpage-transform@<:@=SED-SCRIPT@:>@],
 [apply SED-SCRIPT @<:@s/^/mingw-/@:>@ to installed manpage names])]
[AS_HELP_STRING([--disable-mingw-manpage-transform],
 [@<:@DEFAULT@:>@ don't transform installed manpage names])],
 [case ${enableval} in
    yes) mingw_manpage_transform='s,^,mingw-,' ;;
     no) mingw_manpage_transform='s,x,x,' ;;
      *) mingw_manpage_transform=${enableval} ;;
  esac])
 AC_SUBST([mingw_manpage_transform],[${mingw_manpage_transform-'s,x,x,'}])dnl
])# MINGW_AC_MANPAGE_TRANSFORM

# $RCSfile$: end of file: vim: ft=config

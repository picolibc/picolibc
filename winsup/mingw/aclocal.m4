# generated automatically by aclocal 1.9.6 -*- Autoconf -*-

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

dnl This provides configure definitions used by all the winsup
dnl configure.in files.

# FIXME: We temporarily define our own version of AC_PROG_CC.  This is
# copied from autoconf 2.12, but does not call AC_PROG_CC_WORKS.  We
# are probably using a cross compiler, which will not be able to fully
# link an executable.  This should really be fixed in autoconf
# itself.

AC_DEFUN([LIB_AC_PROG_CC_GNU],
[AC_CACHE_CHECK(whether we are using GNU C, ac_cv_prog_gcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if AC_TRY_COMMAND(${CC-cc} -E conftest.c) | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi])])

AC_DEFUN([LIB_AC_PROG_CC],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_TOOL(CC, gcc, gcc)
: ${CC:=gcc}
AC_PROG_CC
test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
])

AC_DEFUN([LIB_AC_PROG_CXX],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_TOOL(CXX, g++, g++)
if test -z "$CXX"; then
  AC_CHECK_TOOL(CXX, g++, c++, , , )
  : ${CXX:=g++}
  AC_PROG_CXX
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi

CXXFLAGS='$(CFLAGS)'
])


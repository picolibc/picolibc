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
AC_INIT([Cygwin],[0],[cygwin@cygwin.com],[cygwin],[https://cygwin.com])
AC_CONFIG_AUX_DIR(..)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([dejagnu foreign no-define no-dist subdir-objects -Wall -Wno-portability -Wno-extra-portability])
AM_SILENT_RULES([yes])

realdirpath() {
    test -z "$1" && return 1
    (cd "$1" 2>/dev/null && pwd)
    if test $? -ne 0; then
	return 1
    else
	return 0
    fi
}

winsup_srcdir="$(realdirpath $srcdir)"
target_builddir="$(realdirpath ..)"

AC_NO_EXECUTABLES

AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AM_PROG_AS

AC_LANG(C)
AC_LANG(C++)

AC_ARG_WITH([cross-bootstrap],[AS_HELP_STRING([--with-cross-bootstrap],[do not build programs using the MinGW toolchain or check for MinGW libraries (useful for bootstrapping a cross-compiler)])],[],[with_cross_bootstrap=no])

AC_CYGWIN_INCLUDES

AC_CONFIG_HEADER(cygwin/config.h)

AC_CHECK_TOOL(AR, ar, ar)
AC_CHECK_TOOL(AS, as, as)
AC_CHECK_TOOL(DLLTOOL, dlltool, dlltool)
AC_CHECK_TOOL(LD, ld, ld)
AC_CHECK_TOOL(NM, nm, nm)
AC_CHECK_TOOL(OBJCOPY, objcopy, objcopy)
AC_CHECK_TOOL(OBJDUMP, objdump, objdump)
AC_CHECK_TOOL(RANLIB, ranlib, ranlib)
AC_CHECK_TOOL(STRIP, strip, strip)
AC_CHECK_TOOL(WINDRES, windres, windres)

AC_ARG_ENABLE(debugging,
[ --enable-debugging		Build a cygwin DLL which has more consistency checking for debugging],
[case "${enableval}" in
yes)	 AC_DEFINE([DEBUGGING],[1],[Define if DEBUGGING support is requested.]) ;;
no)	 ;;
esac
])

case "$target_cpu" in
   i?86)
		DLL_ENTRY="_dll_entry@12"
		DEF_DLL_ENTRY="dll_entry@12"
		DIN_FILE="i686.din"
		TLSOFFSETS_H="tlsoffsets.h"
		;;
   x86_64)
		DLL_ENTRY="dll_entry"
		DEF_DLL_ENTRY="dll_entry"
		DIN_FILE="x86_64.din"
		TLSOFFSETS_H="tlsoffsets64.h"
		;;
   *)		AC_MSG_ERROR([Invalid target processor "$target_cpu"]) ;;
esac

AC_SUBST(DLL_ENTRY)
AC_SUBST(DEF_DLL_ENTRY)
AC_SUBST(DIN_FILE)
AC_SUBST(TLSOFFSETS_H)

AM_CONDITIONAL(TARGET_X86_64, [test $target_cpu = "x86_64"])

AC_CHECK_PROGS([DOCBOOK2XTEXI], [docbook2x-texi db2x_docbook2texi], [true])

if test "x$with_cross_bootstrap" != "xyes"; then
    AC_CHECK_PROGS(MINGW_CXX, ${target_cpu}-w64-mingw32-g++)
    test -n "$MINGW_CXX" || AC_MSG_ERROR([no acceptable MinGW g++ found in \$PATH])
    AC_CHECK_PROGS(MINGW_CC, ${target_cpu}-w64-mingw32-gcc)
    test -n "$MINGW_CC" || AC_MSG_ERROR([no acceptable MinGW gcc found in \$PATH])
fi
AM_CONDITIONAL(CROSS_BOOTSTRAP, [test "x$with_cross_bootstrap" != "xyes"])

AC_EXEEXT

AC_CHECK_LIB([bfd], [bfd_init], [true],
	     AC_MSG_WARN([Not building dumper.exe since some required libraries or headers are missing]))

AM_CONDITIONAL(BUILD_DUMPER, [test "x$ac_cv_lib_bfd_bfd_init" = "xyes"])

AC_CONFIG_FILES([
    Makefile
    cygwin/Makefile
    cygserver/Makefile
    doc/Makefile
    utils/Makefile
    utils/mingw/Makefile
    testsuite/Makefile
    testsuite/cygrun/Makefile
])

AC_OUTPUT

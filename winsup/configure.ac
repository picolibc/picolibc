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

AC_CONFIG_HEADERS([cygwin/config.h])

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
[AS_HELP_STRING([--enable-debugging],[Build a cygwin DLL which has more consistency checking for debugging])],
[case "${enableval}" in
yes)	 AC_DEFINE([DEBUGGING],[1],[Define if DEBUGGING support is requested.]) ;;
no)	 ;;
esac
])

DLL_ENTRY="dll_entry"

case "$target_cpu" in
   x86_64)	;;
   *)		AC_MSG_ERROR([Invalid target processor "$target_cpu"]) ;;
esac

AC_SUBST(DLL_ENTRY)

AM_CONDITIONAL(TARGET_X86_64, [test $target_cpu = "x86_64"])

AC_ARG_ENABLE(doc,
	      [AS_HELP_STRING([--disable-doc], [do not build documentation])],,
	      enable_doc=yes)
AM_CONDITIONAL(BUILD_DOC, [test $enable_doc != "no"])

AC_CHECK_PROGS([DOCBOOK2XTEXI], [docbook2x-texi db2x_docbook2texi])
if test -z "$DOCBOOK2XTEXI" ; then
    if test "x$enable_doc" != "xno"; then
        AC_MSG_ERROR([docbook2texi is required to build documentation])
    else
        unset DOCBOOK2XTEXI
        AM_MISSING_PROG([DOCBOOK2XTEXI], [docbook2texi])
    fi
fi

AC_CHECK_PROGS([XMLTO], [xmlto])
if test -z "$XMLTO"; then
    if test "x$enable_doc" != "xno"; then
        AC_MSG_ERROR([xmlto is required to build documentation])
    else
        unset XMLTO
        AM_MISSING_PROG([XMLTO], [xmlto])
    fi
fi

if test "x$with_cross_bootstrap" != "xyes"; then
    AC_CHECK_PROGS(MINGW_CXX, ${target_cpu}-w64-mingw32-g++)
    test -n "$MINGW_CXX" || AC_MSG_ERROR([no acceptable MinGW g++ found in \$PATH])
    AC_CHECK_PROGS(MINGW_CC, ${target_cpu}-w64-mingw32-gcc)
    test -n "$MINGW_CC" || AC_MSG_ERROR([no acceptable MinGW gcc found in \$PATH])
fi
AM_CONDITIONAL(CROSS_BOOTSTRAP, [test "x$with_cross_bootstrap" != "xyes"])

AC_EXEEXT

AC_ARG_ENABLE([dumper],
	      [AS_HELP_STRING([--disable-dumper], [do not build the 'dumper' utility])],
	      [build_dumper=$enableval],
	      [build_dumper=yes])

AM_CONDITIONAL(BUILD_DUMPER, [test "x$build_dumper" = "xyes"])

# libbfd.a doesn't have a pkgconfig file, so we guess what it's dependencies
# are, based on what's present in the build environment
BFD_LIBS="-lintl -liconv -liberty -lz"
AC_CHECK_LIB([sframe], [sframe_decode], [BFD_LIBS="${BFD_LIBS} -lsframe"])
AC_CHECK_LIB([zstd], [ZSTD_isError], [BFD_LIBS="${BFD_LIBS} -lzstd"])
AC_SUBST([BFD_LIBS])

AC_CONFIG_FILES([
    Makefile
    cygwin/Makefile
    cygserver/Makefile
    doc/Makefile
    utils/Makefile
    utils/mingw/Makefile
    testsuite/Makefile
    testsuite/mingw/Makefile
])

AC_OUTPUT

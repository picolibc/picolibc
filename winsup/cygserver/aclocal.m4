# generated automatically by aclocal 1.11.6 -*- Autoconf -*-

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software Foundation,
# Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

dnl This provides configure definitions used by all the cygwin
dnl configure.in files.

AC_DEFUN([AC_WINDOWS_HEADERS],[
AC_ARG_WITH(
    [windows-headers],
    [AS_HELP_STRING([--with-windows-headers=DIR],
		    [specify where the windows includes are located])],
    [test -z "$withval" && AC_MSG_ERROR([must specify value for --with-windows-headers])]
)
])

AC_DEFUN([AC_WINDOWS_LIBS],[
AC_ARG_WITH(
    [windows-libs],
    [AS_HELP_STRING([--with-windows-libs=DIR],
		    [specify where the windows libraries are located])],
    [test -z "$withval" && AC_MSG_ERROR([must specify value for --with-windows-libs])]
)
windows_libdir=$(realdirpath "$with_windows_libs")
if test -z "$windows_libdir"; then
    windows_libdir=$(realdirpath $(${ac_cv_prog_CC:-$CC} -xc /dev/null  -Wl,--verbose=1 -lntdll 2>&1 | sed -rn 's%^.*\s(\S+)/libntdll\..*succeeded%\1%p'))
    if test -z "$windows_libdir"; then
	AC_MSG_ERROR([cannot find windows library files])
    fi
fi
AC_SUBST(windows_libdir)
]
)

AC_DEFUN([AC_CYGWIN_INCLUDES], [
addto_CPPFLAGS -nostdinc
: ${ac_cv_prog_CXX:=$CXX}
: ${ac_cv_prog_CC:=$CC}

cygwin_headers=$(realdirpath "$winsup_srcdir/cygwin/include")
if test -z "$cygwin_headers"; then
    AC_MSG_ERROR([cannot find $winsup_srcdir/cygwin/include directory])
fi

newlib_headers=$(realdirpath $winsup_srcdir/../newlib/libc/include)
if test -z "$newlib_headers"; then
    AC_MSG_ERROR([cannot find newlib source directory: $winsup_srcdir/../newlib/libc/include])
fi
newlib_headers="$target_builddir/newlib/targ-include $newlib_headers"

if test -n "$with_windows_headers"; then
    if test -e "$with_windows_headers/windef.h"; then
	windows_headers="$with_windows_headers"
    else
	AC_MSG_ERROR([cannot find windef.h in specified --with-windows-headers path: $saw_windows_headers]);
    fi
elif test -d "$winsup_srcdir/w32api/include/windef.h"; then
    windows_headers="$winsup_srcdir/w32api/include"
else
    windows_headers=$(cd $($ac_cv_prog_CC -xc /dev/null -E -include windef.h 2>/dev/null | sed -n 's%^# 1 "\([^"]*\)/windef\.h".*$%\1%p' | head -n1) 2>/dev/null && pwd)
    if test -z "$windows_headers" -o ! -d "$windows_headers"; then
	AC_MSG_ERROR([cannot find windows header files])
    fi
fi
CC=$ac_cv_prog_CC
CXX=$ac_cv_prog_CXX
export CC
export CXX
AC_SUBST(windows_headers)
AC_SUBST(newlib_headers)
AC_SUBST(cygwin_headers)
])

AC_DEFUN([AC_CONFIGURE_ARGS], [
configure_args=X
for f in $ac_configure_args; do
    case "$f" in
	*--srcdir*) ;;
	*) configure_args="$configure_args $f" ;;
    esac
done
configure_args=$(/usr/bin/expr "$configure_args" : 'X \(.*\)')
AC_SUBST(configure_args)
])

AC_SUBST(target_builddir)
AC_SUBST(winsup_srcdir)


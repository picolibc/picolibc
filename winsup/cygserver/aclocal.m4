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

AC_DEFUN([AC_CYGWIN_INCLUDES], [
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

INCLUDES="-I${srcdir}/../cygwin -I${target_builddir}/winsup/cygwin"
INCLUDES="${INCLUDES} -isystem ${cygwin_headers}"
for h in ${newlib_headers}; do
    INCLUDES="${INCLUDES} -isystem $h"
done
AC_SUBST(INCLUDES)
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


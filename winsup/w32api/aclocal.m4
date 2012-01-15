# aclocal.m4 for MinGW/Cygwin w32api package.  -*- Autoconf -*-
#
# Definitions of autoconf macros used to implement the configure
# script for the w32api package.
#
# $Id$
#
# Written by Keith Marshall <keithmarshall@users.sourceforge.net>
# Copyright (C) 2011, 2012, MinGW Project
#
# -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# -----------------------------------------------------------------------------
#

# MINGW_AC_CONFIG_SRCDIR( VERSION_TAG, UNIQUE_FILE )
# --------------------------------------------------
# Wrapper for AC_CONFIG_SRCDIR; in addition to checking for a
# unique file reference within the source tree, it resolves the
# definition for PACKAGE_VERSION, based on a tagged definition
# within that file.
#
AC_DEFUN([MINGW_AC_CONFIG_SRCDIR],
[AC_CONFIG_SRCDIR([$2])
 AC_MSG_CHECKING([package version])
 PACKAGE_VERSION=`awk '$[2] == "'"$1"'" { print $[3] }' ${srcdir}/$2`
 AC_MSG_RESULT([$PACKAGE_VERSION])dnl
])# MINGW_AC_CONFIG_SRCDIR

# MINGW_AC_RUNTIME_SRCDIR
# -----------------------
# Attempt to identify the locations of the C runtime sources.
# Accept explicit locations specified by the user, as arguments
# to --with-host-srcdir and --with-libc-srcdir; if unspecified,
# fall back to standard locations relative to ${srcdir}.  For a
# MinGW build, identification of an appropriate location will be
# determined by presence of include/_mingw.h in host-srcdir, and
# lib-srcdir is not required; for a Cygwin build, host-srcdir is
# identified by the presence of include/cygwin/cygwin_dll.h, and
# presence of include/newlib.h is also required to identify the
# libc-srcdir location.  This is naive; however, AC_CHECK_HEADER
# cannot be used because the system's runtime headers may result
# in false identification of unsuitable locations.
#
AC_DEFUN([MINGW_AC_RUNTIME_SRCDIR],
[AC_REQUIRE([MINGW_AC_HOST_SRCDIR])dnl
 AC_REQUIRE([MINGW_AC_LIBC_SRCDIR])dnl
 AS_IF([(test "x${with_host_srcdir}" = xMISSING || test "x${with_libc_srcdir}" = xMISSING)],
 [AC_MSG_WARN([the location of required runtime headers cannot be established])
  AC_MSG_WARN([please correct this omission before running configure again])
  AC_MSG_RESULT
  AC_MSG_ERROR([unable to continue until this issue is resolved])
 ])dnl
])# MINGW_AC_RUNTIME_SRCDIR

# MINGW_AC_HOST_SRCDIR
# --------------------
# Helper macro, AC_REQUIREd by MINGW_AC_RUNTIME_SRCDIR.
# This establishes the --with-host-srcdir reference for identification
# of runtime header path; it is never called directly by configure.ac
#
AC_DEFUN([MINGW_AC_HOST_SRCDIR],
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
 AC_ARG_WITH([host-srcdir],
 [AS_HELP_STRING([--with-host-srcdir=DIR],
  [locate host-specific runtime library sources in DIR [SRCDIR/../mingw*]
   ([SRCDIR/../cygwin] when HOST == Cygwin)
  ])dnl
 ],[],[with_host_srcdir=NONE])
 AS_CASE([${host_os}],
  [*cygwin*],[ac_dir="cygwin" ac_file="cygwin/cygwin_dll"],
  [ac_dir="mingw" ac_file="_mingw"])
 MINGW_AC_CHECK_RUNTIME_SRCDIR([host],[host runtime])
 EXTRA_INCLUDES="-I ${with_host_srcdir}/include"
])# MINGW_AC_HOST_SRCDIR

# MINGW_AC_LIBC_SRCDIR
# --------------------
# Helper macro, AC_REQUIREd by MINGW_AC_RUNTIME_SRCDIR.
# This establishes the --with-libc-srcdir reference for identification
# of runtime header path; it is never called directly by configure.ac
#
AC_DEFUN([MINGW_AC_LIBC_SRCDIR],
[AC_ARG_WITH([libc-srcdir],
 [AS_HELP_STRING([--with-libc-srcdir=DIR],
  [locate additional libc sources in DIR [SRCDIR/../../newlib/libc]
   (required only when HOST == Cygwin)
  ])dnl
 ],[],[with_libc_srcdir=NONE])
 AC_MSG_CHECKING([whether additional runtime headers are required])
 AS_CASE([${host_os}],
 [*cygwin*],
  [AC_MSG_RESULT([libc (newlib)])
   ac_dir="../newlib/libc" ac_file="newlib"
   MINGW_AC_CHECK_RUNTIME_SRCDIR([libc],[libc runtime])
   AS_IF([test "x${with_libc_srcdir}" != xNONE],
   [EXTRA_INCLUDES="-I ${with_libc_srcdir}/include ${EXTRA_INCLUDES}"
   ])dnl
  ],dnl
 [AC_MSG_RESULT([no])
 ])dnl
])# MINGW_AC_LIBC_SRCDIR

# MINGW_AC_CHECK_RUNTIME_SRCDIR( KEY, CATEGORY )
# ----------------------------------------------
# Helper macro, invoked by each of MINGW_AC_HOST_SRCDIR and
# MINGW_AC_LIBC_SRCDIR, with KEY passed as 'host' or as 'libc'
# respectively; it establishes and verifies the reference path
# for headers of the class described by CATEGORY, assigning
# the result to the shell variable 'with_KEY_srcdir'.
#
# As is the case for each of the preceding macros, which may
# call it, MINGW_AC_CHECK_RUNTIME_SRCDIR is never invoked
# directly by configure.ac
#
AC_DEFUN([MINGW_AC_CHECK_RUNTIME_SRCDIR],
[AS_IF([test "x${with_$1_srcdir}" = xNONE],
 [AC_MSG_CHECKING([include path for $2 headers])
  for with_$1_srcdir in ${srcdir}/../${ac_dir}*; do
    test -f "${with_$1_srcdir}/include/${ac_file}.h" && break
    with_$1_srcdir=MISSING
  done
  AS_IF([test "x${with_$1_srcdir}" = xMISSING],
  [AC_MSG_RESULT([none found])
   AC_MSG_RESULT
   AC_MSG_WARN([source directory containing include/${ac_file}.h not found])
   AC_MSG_WARN([ensure $2 sources are installed at \${top_srcdir}/../${ac_dir}*])
   AC_MSG_WARN([or use --with-$1-srcdir=DIR to specify an alternative])
   AC_MSG_RESULT
  ],dnl
  [case "${with_$1_srcdir}" in
     "${srcdir}/"*) with_$1_srcdir="`echo "${with_$1_srcdir}" \
       | sed s,"^${srcdir}/",'${top_srcdir}/',`" ;;
   esac
   AC_MSG_RESULT([${with_$1_srcdir}/include])dnl
  ])dnl
 ],dnl
 [AC_MSG_CHECKING([for ${ac_file}.h in ${with_$1_srcdir}/include])
  AS_IF([test -f "${with_$1_srcdir}/include/${ac_file}.h"],
  [AC_MSG_RESULT([yes])
   case "${with_$1_srcdir}" in /*) ;;
     *) with_$1_srcdir='${top_builddir}/'"${with_$1_srcdir}" ;;
   esac
  ],dnl
  [AC_MSG_RESULT([no])
   AC_MSG_RESULT
   AC_MSG_WARN([the nominated directory, ${with_$1_srcdir}])
   AC_MSG_WARN([does not appear to contain valid $2 source code])
   AC_MSG_WARN([(file '${with_$1_srcdir}/include/${ac_file}.h' is not present)])
   AC_MSG_RESULT
   with_$1_srcdir=MISSING
  ])dnl
 ])dnl
])# MINGW_AC_CHECK_RUNTIME_SRCDIR

# $RCSfile$: end of file: vim: ft=config

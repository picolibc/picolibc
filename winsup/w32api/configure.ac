# Process this file with autoconf to produce a configure script.
#
# This file is part of a free win32 library
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

AC_PREREQ([2.59])
#
# Extract current release version from include/w32api.h at autoconf time.
#
m4_define([__ver__],dnl
m4_esyscmd([awk '$2=="__W32API_VERSION"{printf "%s","v"$3}' include/w32api.h]))

AC_INIT([MS-Windows API],[__ver__],[http://mingw.org/reporting_bugs],[w32api])
MINGW_AC_CONFIG_SRCDIR([__W32API_VERSION],[include/w32api.h])

AC_ARG_VAR([RELEASE],[release serial number for current package version])
AC_SUBST([PACKAGE_RELEASE],[${RELEASE-"1"}])

AC_CANONICAL_SYSTEM

# Identify commands which are to be used for installation.
#
AC_PROG_MKDIR_P
AC_PROG_INSTALL

# Check for CC and CFLAGS
# FIXME: why do we not just use AC_PROG_CC here?  AC_CHECK_TOOL
# may be better, in this instance, because it avoids the check for
# ability to create executables, (which we don't actually need).  It
# does also cost us a few other checks, such as identification of the
# proper object file extension, OBJEXT, but that may be a price which
# is justifiable, since we may need to run this in a partially built
# compiler environment, before the compiler is capable of creating
# executables; since we don't actually need that capability, the
# check for it, in AC_PROG_CC, could hurt.
#
AC_CHECK_TOOL([CC], [gcc], [gcc])
AC_SUBST([CFLAGS], [${CFLAGS-"-O2 -g"}])
AC_SUBST([EXTRA_INCLUDES], [MINGW_AC_RUNTIME_SRCDIR])

# FIXME: What is the purpose of this? I think it should go away.
#
AC_SUBST([with_cross_host])

# Check for various tools
#
AC_CHECK_TOOL([AS],      [as],      [as])
AC_CHECK_TOOL([AR],      [ar],      [ar])
AC_CHECK_TOOL([RANLIB],  [ranlib],  [ranlib])
AC_CHECK_TOOL([DLLTOOL], [dlltool], [dlltool])
AC_CHECK_TOOL([WINDRES], [windres], [windres])
AC_CHECK_TOOL([LD],      [ld],      [ld])

# Create makefiles
#
AC_CONFIG_FILES([Makefile Makefile.comm])
AC_CONFIG_FILES([lib/Makefile lib/ddk/Makefile lib/directx/Makefile])
AC_OUTPUT

# configure.ac: end of file

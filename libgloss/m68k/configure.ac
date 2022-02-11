# Copyright (c) 1995, 1996 Cygnus Support
#
# The authors hereby grant permission to use, copy, modify, distribute,
# and license this software and its documentation for any purpose, provided
# that existing copyright notices are retained in all copies and that this
# notice is included verbatim in any distributions. No written agreement,
# license, or royalty fee is required for any of the authorized uses.
# Modifications to this software may be copyrighted by their authors
# and need not follow the licensing terms described here, provided that
# the new terms are clearly indicated on the first page of each file where
# they apply.
#
# process this file with autoconf to produce a configure script.
AC_INIT(crt0.S)

AC_CONFIG_AUX_DIR(../..)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AC_PROG_INSTALL

LIB_AC_PROG_CC

AS=${AS-as}
AC_SUBST(AS)
AR=${AR-ar}
AC_SUBST(AR)
LD=${LD-ld}
AC_SUBST(LD)
AC_PROG_RANLIB
LIB_AM_PROG_AS

AC_NO_EXECUTABLES

AC_MSG_CHECKING([target cpu family])
TARGET=m68k
AC_PREPROC_IFELSE([AC_LANG_PROGRAM([#ifndef __mcoldfire__
		#error we are not coldfire
		#endif])],
	       TARGET="cf",)
AC_PREPROC_IFELSE([AC_LANG_PROGRAM([#ifndef __mfido__
		#error we are not fido
		#endif])],
	       TARGET="fido",)
AC_MSG_RESULT($TARGET)
AC_SUBST(TARGET)
     
host_makefile_frag=${srcdir}/../config/default.mh
target_makefile_frag=${srcdir}/../config/default.mt

dnl We have to assign the same value to other variables because autoconf
dnl doesn't provide a mechanism to substitute a replacement keyword with
dnl arbitrary data or pathnames.
dnl
host_makefile_frag_path=$host_makefile_frag
AC_SUBST(host_makefile_frag_path)
AC_SUBST_FILE(host_makefile_frag)
target_makefile_frag_path=$target_makefile_frag
AC_SUBST(target_makefile_frag_path)
AC_SUBST_FILE(target_makefile_frag)

AM_ENABLE_MULTILIB(, ../..)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

#
# Copyright (C) 2006 KPIT Cummins
# Copyright (C) 2009 Conny Marco Menebröcker
# All rights reserved.
#
# Redistribution and use in source and binary forms is permitted
# provided that the above copyright notice and following paragraph are
# duplicated in all such forms.
#
# This file is distributed WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#
# Process this file with autoconf to produce a configure script.
#
AC_PREREQ(2.59)
AC_INIT(crt0.S)

if test "${enable_shared}" = "yes" ; then
    echo "Shared libraries not supported for cross compiling, ignored"
fi

if test "$srcdir" = "." ; then
  if test "${with_target_subdir}" != "." ; then
    libgloss_topdir="${srcdir}/${with_multisrctop}../../.."
  else
    libgloss_topdir="${srcdir}/${with_multisrctop}../.."
  fi
else
  libgloss_topdir="${srcdir}/../.."
fi
AC_CONFIG_AUX_DIR($libgloss_topdir)

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


host_makefile_frag=${srcdir}/../config/default.mh
target_makefile_frag=${srcdir}/../config/xc16x.mt

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
AC_SUBST(part_specific_obj)
AC_SUBST(script_list)
AC_SUBST(bsp_list)

AC_CONFIG_FILES([Makefile],
[if test -n "$CONFIG_FILES"; then
  unset ac_file
  . ${libgloss_topdir}/config-ml.in
fi],
srcdir=${srcdir}
target=${target}
with_multisubdir=${with_multisubdir}
ac_configure_args="${ac_configure_args} --enable-multilib"
CONFIG_SHELL=${CONFIG_SHELL-/bin/sh}
libgloss_topdir=${libgloss_topdir}
)
AC_OUTPUT



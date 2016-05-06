dnl This is the newlib/libc/sys/phoenix configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([phoenix.c])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake.
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

AC_CONFIG_SUBDIRS(machine)

PHOENIX_MACH_LIB=machine/lib.${aext}
AC_SUBST(PHOENIX_MACH_LIB)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

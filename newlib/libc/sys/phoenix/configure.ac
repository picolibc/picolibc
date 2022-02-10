dnl This is the newlib/libc/sys/phoenix configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([phoenix.c])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake.
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

if test -n "${machine_dir}"; then
  case ${machine_dir} in
	arm) AC_CONFIG_FILES([machine/arm/Makefile]) ;;
  esac
fi
PHOENIX_MACHINE_DIR=machine/${machine_dir}
AC_SUBST(PHOENIX_MACHINE_DIR)

AM_CONDITIONAL(HAVE_MACHINE_DIR, test x${machine_dir} != x)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

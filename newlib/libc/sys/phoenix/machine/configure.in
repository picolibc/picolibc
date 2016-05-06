dnl This is the newlib/libc/sys configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([arm])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../../..)

NEWLIB_CONFIGURE(../../../..)

if test -n "${machine_dir}"; then
  case ${machine_dir} in
	arm) AC_CONFIG_SUBDIRS(arm) ;;
	esirisc) AC_CONFIG_SUBDIRS(esirisc) ;;
	i386) AC_CONFIG_SUBDIRS(i386) ;;
	powerpc) AC_CONFIG_SUBDIRS(powerpc) ;;
  esac;
fi

CRT0=
if test -n "${machine_dir}"; then
  CRT0=crt0.o
fi
AC_SUBST(CRT0)

AM_CONDITIONAL(HAVE_MACHINE_DIR, test x${machine_dir} != x)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

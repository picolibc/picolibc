dnl This is the newlib/libc/machine/powerpc configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([setjmp.S])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

HAVE_POWERPC_ALTIVEC=no
HAVE_POWERPC_SPE=no
case $host in
  powerpc*-*altivec*) HAVE_POWERPC_ALTIVEC=yes ;;
  powerpc*-*spe*) HAVE_POWERPC_SPE=yes ;;
esac
AM_CONDITIONAL(HAVE_POWERPC_ALTIVEC, [test "$HAVE_POWERPC_ALTIVEC" = yes])
AM_CONDITIONAL(HAVE_POWERPC_SPE, [test "$HAVE_POWERPC_SPE" = yes])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

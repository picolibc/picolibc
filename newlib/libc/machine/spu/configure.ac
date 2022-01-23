dnl This is the newlib/libc/machine/spu configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([Makefile.am])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

AC_MSG_CHECKING([whether the compiler supports __ea])
AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if !defined (__EA32__) && !defined (__EA64__)
# error "__ea not supported"
#endif
]])], [spu_compiler_has_ea=yes], [spu_compiler_has_ea=no])
AM_CONDITIONAL(HAVE_SPU_EA, test x${spu_compiler_has_ea} != xno)
AC_MSG_RESULT($spu_compiler_has_ea)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl This is the newlib/libc/machine/nds32 configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([setjmp.S])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

dnl Use builtin macro to detect if this is for "AndeStar ISA V3m".
AC_NO_EXECUTABLES
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#ifdef __NDS32_ISA_V3M__
warning: This is nds32_isa_v3m.
#endif
]])],
		  [is_nds32_isa_v3m="no"],
		  [is_nds32_isa_v3m="yes"])
AM_CONDITIONAL(IS_NDS32_ISA_V3M, test "$is_nds32_isa_v3m" = "yes")


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

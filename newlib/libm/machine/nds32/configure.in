dnl This is the newlib/libm/machine/nds32 configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([Makefile.am])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake.
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)


dnl Use builtin macro to detect if FPU extension support is on.
AC_NO_EXECUTABLES
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#if (__NDS32_EXT_FPU_SP__)
warning: Has nds32 FPU SP extension support
#endif
]])],
                  [has_nds32_fpu_sp="no"],
		  [has_nds32_fpu_sp="yes"])
AM_CONDITIONAL(HAS_NDS32_FPU_SP, test "$has_nds32_fpu_sp" = "yes")

AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#if (__NDS32_EXT_FPU_DP__)
warning: Has nds32 FPU DP extension support
#endif
]])],
                  [has_nds32_fpu_dp="no"],
		  [has_nds32_fpu_dp="yes"])
AM_CONDITIONAL(HAS_NDS32_FPU_DP, test "$has_nds32_fpu_dp" = "yes")

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

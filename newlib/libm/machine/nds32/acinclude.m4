if test "${libm_machine_dir}" = "nds32"; then
  dnl Use builtin macro to detect if FPU extension support is on.
  AC_CACHE_CHECK([for nds32 FPU SP extension], newlib_cv_nds32_fpu_sp, [dnl
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if (__NDS32_EXT_FPU_SP__)
# error "Has nds32 FPU SP extension support"
#endif
]])], [newlib_cv_nds32_fpu_sp="no"], [newlib_cv_nds32_fpu_sp="yes"])])

  AC_CACHE_CHECK([for nds32 FPU DP extension], newlib_cv_nds32_fpu_dp, [dnl
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if (__NDS32_EXT_FPU_DP__)
# error "Has nds32 FPU DP extension support"
#endif
]])], [newlib_cv_nds32_fpu_dp="no"], [newlib_cv_nds32_fpu_dp="yes"])])
fi

AM_CONDITIONAL(HAS_NDS32_FPU_SP, test "$newlib_cv_nds32_fpu_sp" = "yes")
AM_CONDITIONAL(HAS_NDS32_FPU_DP, test "$newlib_cv_nds32_fpu_dp" = "yes")

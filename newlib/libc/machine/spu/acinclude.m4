if test "${machine_dir}" = "spu"; then
  AC_CACHE_CHECK([whether the compiler supports __ea], newlib_cv_spu_compiler_has_ea, [dnl
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if !defined (__EA32__) && !defined (__EA64__)
# error "__ea not supported"
#endif
]])], [newlib_cv_spu_compiler_has_ea=yes], [newlib_cv_spu_compiler_has_ea=no])])
fi
AM_CONDITIONAL(HAVE_SPU_EA, test "$newlib_cv_spu_compiler_has_ea" = yes)

if test "${machine_dir}" = "spu"; then
  AC_MSG_CHECKING([whether the compiler supports __ea])
  AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if !defined (__EA32__) && !defined (__EA64__)
# error "__ea not supported"
#endif
]])], [spu_compiler_has_ea=yes], [spu_compiler_has_ea=no])
  AC_MSG_RESULT($spu_compiler_has_ea)
fi
AM_CONDITIONAL(HAVE_SPU_EA, test x${spu_compiler_has_ea} != xno)

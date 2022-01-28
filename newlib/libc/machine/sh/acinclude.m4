if test "${machine_dir}" = "sh"; then
  AC_CACHE_CHECK([for SH5 (64-bit)], newlib_cv_sh64, [dnl
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if !defined(__SH5__)
# error "not SH5"
#endif
]])], [newlib_cv_sh64=yes], [newlib_cv_sh64=no])])
fi

AM_CONDITIONAL(SH64, [test "$newlib_cv_sh64" = yes])

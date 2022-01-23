if test "${machine_dir}" = "sh"; then
  AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#if !defined(__SH5__)
# error "not SH5"
#endif
]])], [have_sh64=yes], [have_sh64=no])
fi

AM_CONDITIONAL(SH64, [test "$have_sh64" = yes])

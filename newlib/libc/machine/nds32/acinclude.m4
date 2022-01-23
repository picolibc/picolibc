if test "${machine_dir}" = "nds32"; then
  dnl Use builtin macro to detect if this is for "AndeStar ISA V3m".
  AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#ifdef __NDS32_ISA_V3M__
# error "This is nds32_isa_v3m."
#endif
]])], [is_nds32_isa_v3m="no"], [is_nds32_isa_v3m="yes"])
fi

AM_CONDITIONAL(IS_NDS32_ISA_V3M, test "$is_nds32_isa_v3m" = "yes")

MN10300_SCRIPT_LIST=
MN10300_BSP_LIST=
case "${target}" in
  mn10300-*elf)
    MN10300_SCRIPT_LIST="eval sim asb2303 asb2305"
    MN10300_BSP_LIST="libeval.a libcygmon.a"
    ;;
  *)
    MN10300_SCRIPT_LIST="eval sim"
    MN10300_BSP_LIST="libeval.a"
    ;;
esac
AC_SUBST(MN10300_SCRIPT_LIST)
AC_SUBST(MN10300_BSP_LIST)

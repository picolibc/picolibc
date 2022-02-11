dnl Don't build crt0 for moxiebox, which provides crt0 for us.
case "${target}" in
  moxie-*-moxiebox*)
    MOXIE_BUILD_CRT0_TRUE='#'
    MOXIE_BUILD_CRT0_FALSE=
    ;;
  *)
    MOXIE_BUILD_CRT0_TRUE=
    MOXIE_BUILD_CRT0_FALSE='#'
    ;;
esac
AC_SUBST(MOXIE_BUILD_CRT0_TRUE)
AC_SUBST(MOXIE_BUILD_CRT0_FALSE)

M68K_TARGET=m68k
case "${target}" in
  fido-*-* | m68*-*-*)
    AC_MSG_CHECKING([target cpu family])
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
#ifndef __mcoldfire__
#error we are not coldfire
#endif])], M68K_TARGET="cf")
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([
#ifndef __mfido__
#error we are not fido
#endif])], M68K_TARGET="fido")
    AC_MSG_RESULT($M68K_TARGET)
    ;;
esac
AC_SUBST(M68K_TARGET)

WINCE_STUB_LIBS=
case "${target}" in
  *arm*-wince-pe) WINCE_STUB_LIBS='-lsslsock' ;;
esac
AC_SUBST(WINCE_STUB_LIBS)

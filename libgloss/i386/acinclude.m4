I386_CPPFLAGS=
case "${target}" in
  i[[3456]]86-*-coff)
    I386_CPPFLAGS="-DCOFF"
    ;;
  i[[3456]]86-*-aout)
    I386_CPPFLAGS="-DAOUT"
    ;;
esac
AC_SUBST(I386_CPPFLAGS)

HAVE_POWERPC_ALTIVEC=no
HAVE_POWERPC_SPE=no
if test "${machine_dir}" = "powerpc"; then
  case $host in
    powerpc*-*altivec*) HAVE_POWERPC_ALTIVEC=yes ;;
    powerpc*-*spe*) HAVE_POWERPC_SPE=yes ;;
  esac
fi
AM_CONDITIONAL(HAVE_POWERPC_ALTIVEC, [test "$HAVE_POWERPC_ALTIVEC" = yes])
AM_CONDITIONAL(HAVE_POWERPC_SPE, [test "$HAVE_POWERPC_SPE" = yes])

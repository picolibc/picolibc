SPARC_CPU=SPARC
case ${target_cpu} in
sparclite*) SPARC_CPU=SLITE ;;
sparclet*) SPARC_CPU=SPLET ;;
sparc64*) SPARC_CPU=SPARC64 ;;
sparc86x*) SPARC_CPU=SLITE ;;
esac
AC_SUBST(SPARC_CPU)

SPARC_CYGMONLDSCRIPTTEMPL=${srcdir}/sparc/cygmon.ld.src
case ${target_cpu} in
sparc64*) SPARC_CYGMONLDSCRIPTTEMPL=${srcdir}/sparc/cygmon-sparc64-ld.src ;;
sparclet-*-aout*) SPARC_CYGMONLDSCRIPTTEMPL=${srcdir}/sparc/cygmon.ld.src; AC_CONFIG_FILES([sparc/libsys/Makefile]) ;;
esac
AC_SUBST(SPARC_CYGMONLDSCRIPTTEMPL)

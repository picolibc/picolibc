dnl Process this file with autoconf to produce a configure script.
AC_INIT(Makefile.in)

AC_PROG_INSTALL

# Command-line options.
# Very limited version of AC_MAINTAINER_MODE.
AC_ARG_ENABLE([maintainer-mode],
  [AC_HELP_STRING([--enable-maintainer-mode],
                 [enable make rules and dependencies not useful (and
                  sometimes confusing) to the casual installer])],
  [case ${enable_maintainer_mode} in
     yes) MAINT='' ;;
     no) MAINT='#' ;;
     *) AC_MSG_ERROR([--enable-maintainer-mode must be yes or no]) ;;
   esac
   maintainer_mode=${enableval}],
  [MAINT='#'])
AC_SUBST([MAINT])dnl

AC_SUBST(datarootdir)
AC_SUBST(docdir)
AC_SUBST(htmldir)
AC_SUBST(pdfdir)

AC_OUTPUT(Makefile)

AC_PREREQ(2.59)dnl
AC_INIT(Makefile.in)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_AUX_DIR(..)

. ${srcdir}/../configure.cygwin

AC_PROG_INSTALL
AC_NO_EXECUTABLES
AC_CANONICAL_SYSTEM

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_LANG(C)

AC_WINDOWS_HEADERS
AC_WINDOWS_LIBS

AC_LANG(C++)

AC_CYGWIN_INCLUDES

case "$with_cross_host" in
  ""|*cygwin*)
    all_host="all_host"
    install_host="install_host"
    ;;
  *)
    all_host=
    install_host=
    ;;
esac

AC_SUBST(all_host)
AC_SUBST(install_host)

AC_CHECK_TOOL(AR, ar, ar)
AC_CHECK_TOOL(AS, as, as)
AC_CHECK_TOOL(DLLTOOL, dlltool, dlltool)
AC_CHECK_TOOL(LD, ld, ld)
AC_CHECK_TOOL(NM, nm, nm)
AC_CHECK_TOOL(OBJCOPY, objcopy, objcopy)
AC_CHECK_TOOL(OBJDUMP, objdump, objdump)
AC_CHECK_TOOL(RANLIB, ranlib, ranlib)
AC_CHECK_TOOL(STRIP, strip, strip)
AC_CHECK_TOOL(WINDRES, windres, windres)

AC_PROG_MAKE_SET

AC_ARG_ENABLE(debugging,
[ --enable-debugging		Build a cygwin DLL which has more consistency checking for debugging],
[case "${enableval}" in
yes)	 AC_DEFINE(DEBUGGING) ;;
no)	 ;;
esac
])

dnl The only time we might want to transform the install names
dnl is for unix x cygwin.  Otherwise we don't.  For now we don't
dnl transform names.

dnl if test "x$cross_compiling" = "xno" -a ; then
dnl   if test "x$program_transform_name" = "xs,x,x,"; then
dnl     program_transform_name=""
dnl   fi
dnl   if test "x$program_transform_name" = "x"; then
dnl     program_transform_name="s,^,$target_alias-,"
dnl   else
dnl     program_transform_name="$program_transform_name -e s,^,$target_alias-,"
dnl   fi
dnl fi

case "$target_cpu" in
   i?86)
		DLL_NAME="cygwin1.dll"
		DLL_ENTRY="_dll_entry@12"
		DEF_DLL_ENTRY="dll_entry@12"
		DIN_FILE="i686.din"
		TLSOFFSETS_H="tlsoffsets.h"
		;;
   x86_64)
		DLL_NAME="cygwin1.dll"
		DLL_ENTRY="dll_entry"
		DEF_DLL_ENTRY="dll_entry"
		DIN_FILE="x86_64.din"
		TLSOFFSETS_H="tlsoffsets64.h"
		;;
   *)		AC_MSG_ERROR(Invalid target processor \"$target_cpu\") ;;
esac

AC_CONFIGURE_ARGS
AC_SUBST(LIBSERVER)
AC_SUBST(DLL_NAME)
AC_SUBST(DLL_ENTRY)
AC_SUBST(DEF_DLL_ENTRY)
AC_SUBST(DIN_FILE)
AC_SUBST(TLSOFFSETS_H)
AC_OUTPUT(Makefile)

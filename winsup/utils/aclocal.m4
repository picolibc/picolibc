dnl aclocal.m4 generated automatically by aclocal 1.3b

dnl Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Check to see if we're running under Win32, without using
# AC_CANONICAL_*.  If so, set output variable EXEEXT to ".exe".
# Otherwise set it to "".

dnl AM_EXEEXT()
dnl This knows we add .exe if we're building in the Cygwin
dnl environment. But if we're not, then it compiles a test program
dnl to see if there is a suffix for executables.
AC_DEFUN(AM_EXEEXT,
[AC_REQUIRE([AM_CYGWIN])
AC_REQUIRE([AM_MINGW32])
AC_MSG_CHECKING([for executable suffix])
AC_CACHE_VAL(am_cv_exeext,
[if test "$CYGWIN" = yes || test "$MINGW32" = yes; then
am_cv_exeext=.exe
else
cat > am_c_test.c << 'EOF'
int main() {
/* Nothing needed here */
}
EOF
${CC-cc} -o am_c_test $CFLAGS $CPPFLAGS $LDFLAGS am_c_test.c $LIBS 1>&5
am_cv_exeext=
for file in am_c_test.*; do
   case $file in
    *.c) ;;
    *.o) ;;
    *) am_cv_exeext=`echo $file | sed -e s/am_c_test//` ;;
   esac
done
rm -f am_c_test*])
test x"${am_cv_exeext}" = x && am_cv_exeext=no
fi
EXEEXT=""
test x"${am_cv_exeext}" != xno && EXEEXT=${am_cv_exeext}
AC_MSG_RESULT(${am_cv_exeext})
AC_SUBST(EXEEXT)])

# Check to see if we're running under Cygwin, without using
# AC_CANONICAL_*.  If so, set output variable CYGWIN to "yes".
# Otherwise set it to "no".

dnl AM_CYGWIN()
AC_DEFUN(AM_CYGWIN,
[AC_CACHE_CHECK(for Cygwin environment, am_cv_cygwin,
[AC_TRY_COMPILE(,[return __CYGWIN32__;],
am_cv_cygwin=yes, am_cv_cygwin=no)
rm -f conftest*])
CYGWIN=
test "$am_cv_cygwin" = yes && CYGWIN=yes])



# Check to see if we're running under Mingw, without using
# AC_CANONICAL_*.  If so, set output variable MINGW32 to "yes".
# Otherwise set it to "no".

dnl AM_MINGW32()
AC_DEFUN(AM_MINGW32,
[AC_CACHE_CHECK(for Mingw32 environment, am_cv_mingw32,
[AC_TRY_COMPILE(,[return __MINGW32__;],
am_cv_mingw32=yes, am_cv_mingw32=no)
rm -f conftest*])
MINGW32=
test "$am_cv_mingw32" = yes && MINGW32=yes])


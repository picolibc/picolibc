dnl This provides configure definitions used by all the winsup
dnl configure.in files.

# FIXME: We temporarily define our own version of AC_PROG_CC.  This is
# copied from autoconf 2.12, but does not call AC_PROG_CC_WORKS.  We
# are probably using a cross compiler, which will not be able to fully
# link an executable.  This should really be fixed in autoconf
# itself.

AC_DEFUN([LIB_AC_PROG_CC_GNU],
[AC_CACHE_CHECK(whether we are using GNU C, ac_cv_prog_gcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if AC_TRY_COMMAND(${CC-cc} -E conftest.c) | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi])])

AC_DEFUN([LIB_AC_PROG_CC],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_TOOL(CC, gcc, gcc)
: ${CC:=gcc}
AC_PROG_CC
test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
])

AC_DEFUN([LIB_AC_PROG_CXX],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_TOOL(CXX, g++, g++)
if test -z "$CXX"; then
  AC_CHECK_TOOL(CXX, g++, c++, , , )
  : ${CXX:=g++}
  AC_PROG_CXX
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi

CXXFLAGS='$(CFLAGS)'
])

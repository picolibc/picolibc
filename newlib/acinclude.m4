dnl This provides configure definitions used by all the newlib
dnl configure.in files.

dnl Basic newlib configury.  This calls basic introductory stuff,
dnl including AM_INIT_AUTOMAKE and AC_CANONICAL_HOST.  It also runs
dnl configure.host.  The only argument is the relative path to the top
dnl newlib directory.

AC_DEFUN(NEWLIB_CONFIGURE,
[
dnl Default to --enable-multilib
AC_ARG_ENABLE(multilib,
[  --enable-multilib         build many library versions (default)],
[case "${enableval}" in
  yes) multilib=yes ;;
  no)  multilib=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for multilib option) ;;
 esac], [multilib=yes])dnl

dnl Support --enable-target-optspace
AC_ARG_ENABLE(target-optspace,
[  --enable-target-optspace  optimize for space],
[case "${enableval}" in
  yes) target_optspace=yes ;;
  no)  target_optspace=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for target-optspace option) ;;
 esac], [target_optspace=])dnl

dnl Support --enable-malloc-debugging - currently only supported for Cygwin
AC_ARG_ENABLE(malloc-debugging,
[  --enable-malloc-debugging indicate malloc debugging requested],
[case "${enableval}" in
  yes) malloc_debugging=yes ;;
  no)  malloc_debugging=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for malloc-debugging option) ;;
 esac], [malloc_debugging=])dnl

dnl Support --enable-newlib-mb
AC_ARG_ENABLE(newlib-mb,
[  --enable-newlib-mb        enable multibyte support],
[case "${enableval}" in
  yes) newlib_mb=yes ;;
  no)  newlib_mb=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-mb option) ;;
 esac], [newlib_mb=no])dnl

dnl We may get other options which we don't document:
dnl --with-target-subdir, --with-multisrctop, --with-multisubdir

test -z "[$]{with_target_subdir}" && with_target_subdir=.

if test "[$]{srcdir}" = "."; then
  if test "[$]{with_target_subdir}" != "."; then
    newlib_basedir="[$]{srcdir}/[$]{with_multisrctop}../$1"
  else
    newlib_basedir="[$]{srcdir}/[$]{with_multisrctop}$1"
  fi
else
  newlib_basedir="[$]{srcdir}/$1"
fi
AC_SUBST(newlib_basedir)

AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(newlib, 1.9.0)

# FIXME: We temporarily define our own version of AC_PROG_CC.  This is
# copied from autoconf 2.12, but does not call AC_PROG_CC_WORKS.  We
# are probably using a cross compiler, which will not be able to fully
# link an executable.  This should really be fixed in autoconf
# itself.

AC_DEFUN(LIB_AC_PROG_CC,
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_PROG(CC, gcc, gcc)
if test -z "$CC"; then
  AC_CHECK_PROG(CC, cc, cc, , , /usr/ucb/cc)
  test -z "$CC" && AC_MSG_ERROR([no acceptable cc found in \$PATH])
fi

AC_PROG_CC_GNU

if test $ac_cv_prog_gcc = yes; then
  GCC=yes
dnl Check whether -g works, even if CFLAGS is set, in case the package
dnl plays around with CFLAGS (such as to build both debugging and
dnl normal versions of a library), tasteless as that idea is.
  ac_test_CFLAGS="${CFLAGS+set}"
  ac_save_CFLAGS="$CFLAGS"
  CFLAGS=
  AC_PROG_CC_G
  if test "$ac_test_CFLAGS" = set; then
    CFLAGS="$ac_save_CFLAGS"
  elif test $ac_cv_prog_cc_g = yes; then
    CFLAGS="-g -O2"
  else
    CFLAGS="-O2"
  fi
else
  GCC=
  test "${CFLAGS+set}" = set || CFLAGS="-g"
fi
])

LIB_AC_PROG_CC

# AC_CHECK_TOOL does AC_REQUIRE (AC_CANONICAL_BUILD).  If we don't
# run it explicitly here, it will be run implicitly before
# NEWLIB_CONFIGURE, which doesn't work because that means that it will
# be run before AC_CANONICAL_HOST.
AC_CANONICAL_BUILD

AC_CHECK_TOOL(AS, as)
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, :)

AC_PROG_INSTALL

AM_MAINTAINER_MODE

# We need AC_EXEEXT to keep automake happy in cygnus mode.  However,
# at least currently, we never actually build a program, so we never
# need to use $(EXEEXT).  Moreover, the test for EXEEXT normally
# fails, because we are probably configuring with a cross compiler
# which can't create executables.  So we include AC_EXEEXT to keep
# automake happy, but we don't execute it, since we don't care about
# the result.
if false; then
  AC_EXEEXT
fi

. [$]{newlib_basedir}/configure.host

case [$]{newlib_basedir} in
/* | [A-Za-z]:[/\\]*) newlib_flagbasedir=[$]{newlib_basedir} ;;
*) newlib_flagbasedir='[$](top_builddir)/'[$]{newlib_basedir} ;;
esac

newlib_cflags="[$]{newlib_cflags} -I"'[$](top_builddir)'"/$1/targ-include -I[$]{newlib_flagbasedir}/libc/include"
case "${host}" in
  *-*-cygwin*)
    newlib_cflags="[$]{newlib_cflags} -I[$]{newlib_flagbasedir}/../winsup/cygwin/include  -I[$]{newlib_flagbasedir}/../winsup/w32api/include"
    ;;
esac

newlib_cflags="[$]{newlib_cflags} -fno-builtin"

NEWLIB_CFLAGS=${newlib_cflags}
AC_SUBST(NEWLIB_CFLAGS)

AC_SUBST(machine_dir)
AC_SUBST(sys_dir)
])

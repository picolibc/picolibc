dnl This provides configure definitions used by all the newlib
dnl configure.in files.

AC_DEFUN([DEF_NEWLIB_MAJOR_VERSION],m4_define([NEWLIB_MAJOR_VERSION],[4]))
AC_DEFUN([DEF_NEWLIB_MINOR_VERSION],m4_define([NEWLIB_MINOR_VERSION],[3]))
AC_DEFUN([DEF_NEWLIB_PATCHLEVEL_VERSION],m4_define([NEWLIB_PATCHLEVEL_VERSION],[0]))
AC_DEFUN([DEF_NEWLIB_VERSION],m4_define([NEWLIB_VERSION],[NEWLIB_MAJOR_VERSION.NEWLIB_MINOR_VERSION.NEWLIB_PATCHLEVEL_VERSION]))

dnl Basic newlib configury.  This calls basic introductory stuff,
dnl including AM_INIT_AUTOMAKE and AC_CANONICAL_HOST.  It also runs
dnl configure.host.  The only argument is the relative path to the top
dnl newlib directory.

AC_DEFUN([NEWLIB_CONFIGURE],
[AC_REQUIRE([DEF_NEWLIB_VERSION])
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

dnl Support --enable-newlib-multithread
AC_ARG_ENABLE(newlib-multithread,
[  --enable-newlib-multithread        enable support for multiple threads],
[case "${enableval}" in
  yes) newlib_multithread=yes ;;
  no)  newlib_multithread=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-multithread option) ;;
 esac], [newlib_multithread=yes])dnl

dnl Support --enable-newlib-iconv
AC_ARG_ENABLE(newlib-iconv,
[  --enable-newlib-iconv     enable iconv library support],
[if test "${newlib_iconv+set}" != set; then
   case "${enableval}" in
     yes) newlib_iconv=yes ;;
     no)  newlib_iconv=no ;;
     *)   AC_MSG_ERROR(bad value ${enableval} for newlib-iconv option) ;;
   esac
 fi], [newlib_iconv=${newlib_iconv}])dnl

dnl Support --enable-newlib-elix-level
AC_ARG_ENABLE(newlib-elix-level,
[  --enable-newlib-elix-level         supply desired elix library level (1-4)],
[case "${enableval}" in
  0)   newlib_elix_level=0 ;;
  1)   newlib_elix_level=1 ;;
  2)   newlib_elix_level=2 ;;
  3)   newlib_elix_level=3 ;;
  4)   newlib_elix_level=4 ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-elix-level option) ;;
 esac], [newlib_elix_level=0])dnl

dnl Support --disable-newlib-io-float
AC_ARG_ENABLE(newlib-io-float,
[  --disable-newlib-io-float disable printf/scanf family float support],
[case "${enableval}" in
  yes) newlib_io_float=yes ;;
  no)  newlib_io_float=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-io-float option) ;;
 esac], [newlib_io_float=yes])dnl

dnl Support --disable-newlib-supplied-syscalls
AC_ARG_ENABLE(newlib-supplied-syscalls,
[  --disable-newlib-supplied-syscalls disable newlib from supplying syscalls],
[case "${enableval}" in
  yes) newlib_may_supply_syscalls=yes ;;
  no)  newlib_may_supply_syscalls=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-supplied-syscalls option) ;;
 esac], [newlib_may_supply_syscalls=yes])dnl

AM_CONDITIONAL(MAY_SUPPLY_SYSCALLS, test x[$]{newlib_may_supply_syscalls} = xyes)

dnl Support --disable-newlib-fno-builtin
AC_ARG_ENABLE(newlib-fno-builtin,
[  --disable-newlib-fno-builtin disable -fno-builtin flag to allow compiler to use builtin library functions],
[case "${enableval}" in
  yes) newlib_fno_builtin=yes ;;
  no)  newlib_fno_builtin=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-fno-builtin option) ;;
 esac], [newlib_fno_builtin=])dnl


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

abs_newlib_basedir=`cd "${newlib_basedir}" && pwd`
AC_SUBST(abs_newlib_basedir)

AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign no-installinfo no-dist no-define subdir-objects 1.15.1])
AM_MAINTAINER_MODE
AM_SILENT_RULES(yes)

AC_NO_EXECUTABLES

AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([AM_PROG_AS])dnl
AC_REQUIRE([AM_PROG_AR])dnl
AC_PROG_RANLIB
AC_CHECK_TOOL(READELF, readelf, :)

dnl We need these programs, but so does Automake.  Require the macros to avoid
dnl expanding them multiple times.
AC_REQUIRE([AC_PROG_INSTALL])dnl
AC_REQUIRE([AC_PROG_AWK])dnl

. [$]{newlib_basedir}/configure.host

NEWLIB_CFLAGS=${newlib_cflags}
AC_SUBST(NEWLIB_CFLAGS)

NO_INCLUDE_LIST=${noinclude}
AC_SUBST(NO_INCLUDE_LIST)

AM_CONDITIONAL(ELIX_LEVEL_0, test x[$]{newlib_elix_level} = x0)
AM_CONDITIONAL(ELIX_LEVEL_1, test x[$]{newlib_elix_level} = x1)
AM_CONDITIONAL(ELIX_LEVEL_2, test x[$]{newlib_elix_level} = x2)
AM_CONDITIONAL(ELIX_LEVEL_3, test x[$]{newlib_elix_level} = x3)
AM_CONDITIONAL(ELIX_LEVEL_4, test x[$]{newlib_elix_level} = x4)

# Emit any target-specific warnings.
if test "x${newlib_msg_warn}" != "x"; then
   AC_MSG_WARN([${newlib_msg_warn}])
fi

# Hard-code OBJEXT.  Normally it is set by AC_OBJEXT, but we
# use oext, which is set in configure.host based on the target platform.
OBJEXT=o

AC_SUBST(OBJEXT)
AC_SUBST(lpfx)

AC_SUBST(libm_machine_dir)
AC_SUBST(machine_dir)
AC_SUBST(shared_machine_dir)
AC_SUBST(sys_dir)
])

dnl This is the newlib/libm configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([libm.texi])

AC_ARG_ENABLE(newlib_hw_fp,
[  --enable-newlib-hw-fp    Turn on hardware floating point math],
[case "${enableval}" in
   yes) newlib_hw_fp=true ;;
   no)  newlib_hw_fp=false ;;
   *) AC_MSG_ERROR(bad value ${enableval} for --enable-newlib-hw-fp) ;;
 esac],[newlib_hw_fp=false])
AM_CONDITIONAL(NEWLIB_HW_FP, test x$newlib_hw_fp = xtrue)

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../..)

NEWLIB_CONFIGURE(..)

dnl We have to enable libtool after NEWLIB_CONFIGURE because if we try and
dnl add it into NEWLIB_CONFIGURE, executable tests are made before the first
dnl line of the macro which fail because appropriate LDFLAGS are not set.
_LT_DECL_SED
_LT_PROG_ECHO_BACKSLASH
if test "${use_libtool}" = "yes"; then
LT_INIT([win32-dll])
fi

AC_TYPE_LONG_DOUBLE
AM_CONDITIONAL(HAVE_LONG_DOUBLE, test x"$ac_cv_type_long_double" = x"yes")

LIBM_MACHINE_LIB=
if test -n "${libm_machine_dir}"; then
  case ${libm_machine_dir} in
    aarch64) AC_CONFIG_SUBDIRS(machine/aarch64) ;;
    arm) AC_CONFIG_SUBDIRS(machine/arm) ;;
    i386) AC_CONFIG_SUBDIRS(machine/i386) ;;
    nds32) AC_CONFIG_SUBDIRS(machine/nds32) ;;
    pru) AC_CONFIG_SUBDIRS(machine/pru) ;;
    spu) AC_CONFIG_SUBDIRS(machine/spu) ;;
    riscv) AC_CONFIG_SUBDIRS(machine/riscv) ;;
    x86_64) AC_CONFIG_SUBDIRS(machine/x86_64) ;;
    powerpc) AC_CONFIG_SUBDIRS(machine/powerpc) ;;
    sparc) AC_CONFIG_SUBDIRS(machine/sparc) ;;
    mips) AC_CONFIG_SUBDIRS(machine/mips) ;;
    *) AC_MSG_ERROR([unsupported libm_machine_dir "${libm_machine_dir}"]) ;;
  esac

  LIBM_MACHINE_DIR=machine/${libm_machine_dir}
  if test "${use_libtool}" = "yes"; then
    LIBM_MACHINE_LIB=${LIBM_MACHINE_DIR}/lib${libm_machine_dir}.${aext}
  else
    LIBM_MACHINE_LIB=${LIBM_MACHINE_DIR}/lib.${aext}
  fi
fi
AM_CONDITIONAL(HAVE_LIBM_MACHINE_DIR, test "x${LIBM_MACHINE_DIR}" != x)
AC_SUBST(LIBM_MACHINE_DIR)
AC_SUBST(LIBM_MACHINE_LIB)

AC_CONFIG_FILES([Makefile math/Makefile mathfp/Makefile common/Makefile complex/Makefile fenv/Makefile])
AC_OUTPUT

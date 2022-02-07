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

AC_TYPE_LONG_DOUBLE
AM_CONDITIONAL(HAVE_LONG_DOUBLE, test x"$ac_cv_type_long_double" = x"yes")

dnl We have to include these unconditionally since machines might want to use
dnl AM_CONDITIONAL in their subdirs.
m4_include([machine/nds32/acinclude.m4])

LIBM_MACHINE_LIB=
if test -n "${libm_machine_dir}"; then
  case ${libm_machine_dir} in
    aarch64) AC_CONFIG_FILES([machine/aarch64/Makefile]) ;;
    arm) AC_CONFIG_FILES([machine/arm/Makefile]) ;;
    i386) AC_CONFIG_FILES([machine/i386/Makefile]) ;;
    nds32) AC_CONFIG_FILES([machine/nds32/Makefile]) ;;
    pru) AC_CONFIG_FILES([machine/pru/Makefile]) ;;
    spu) AC_CONFIG_FILES([machine/spu/Makefile]) ;;
    riscv) AC_CONFIG_FILES([machine/riscv/Makefile]) ;;
    x86_64) AC_CONFIG_FILES([machine/x86_64/Makefile]) ;;
    powerpc) AC_CONFIG_FILES([machine/powerpc/Makefile]) ;;
    sparc) AC_CONFIG_FILES([machine/sparc/Makefile]) ;;
    mips) AC_CONFIG_FILES([machine/mips/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported libm_machine_dir "${libm_machine_dir}"]) ;;
  esac

  LIBM_MACHINE_DIR=machine/${libm_machine_dir}
  LIBM_MACHINE_LIB=${LIBM_MACHINE_DIR}/lib.${aext}
fi
AM_CONDITIONAL(HAVE_LIBM_MACHINE_DIR, test "x${LIBM_MACHINE_DIR}" != x)
AC_SUBST(LIBM_MACHINE_DIR)
AC_SUBST(LIBM_MACHINE_LIB)

AC_CONFIG_FILES([Makefile math/Makefile mathfp/Makefile common/Makefile complex/Makefile fenv/Makefile])
AC_OUTPUT

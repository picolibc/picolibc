dnl We have to include these unconditionally since machines might want to use
dnl AM_CONDITIONAL in their subdirs.
m4_include([libm/machine/nds32/acinclude.m4])

LIBM_MACHINE_LIB=
if test -n "${libm_machine_dir}"; then
  case ${libm_machine_dir} in
    aarch64) AC_CONFIG_FILES([libm/machine/aarch64/Makefile]) ;;
    arm) AC_CONFIG_FILES([libm/machine/arm/Makefile]) ;;
    i386) AC_CONFIG_FILES([libm/machine/i386/Makefile]) ;;
    nds32) AC_CONFIG_FILES([libm/machine/nds32/Makefile]) ;;
    pru) AC_CONFIG_FILES([libm/machine/pru/Makefile]) ;;
    spu) AC_CONFIG_FILES([libm/machine/spu/Makefile]) ;;
    riscv) AC_CONFIG_FILES([libm/machine/riscv/Makefile]) ;;
    x86_64) AC_CONFIG_FILES([libm/machine/x86_64/Makefile]) ;;
    powerpc) AC_CONFIG_FILES([libm/machine/powerpc/Makefile]) ;;
    sparc) AC_CONFIG_FILES([libm/machine/sparc/Makefile]) ;;
    mips) AC_CONFIG_FILES([libm/machine/mips/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported libm_machine_dir "${libm_machine_dir}"]) ;;
  esac

  LIBM_MACHINE_DIR=machine/${libm_machine_dir}
  LIBM_MACHINE_LIB=${LIBM_MACHINE_DIR}/lib.a
fi
AM_CONDITIONAL(HAVE_LIBM_MACHINE_DIR, test "x${LIBM_MACHINE_DIR}" != x)
AC_SUBST(LIBM_MACHINE_DIR)
AC_SUBST(LIBM_MACHINE_LIB)

AC_CONFIG_FILES([libm/Makefile libm/math/Makefile libm/mathfp/Makefile libm/common/Makefile libm/complex/Makefile libm/fenv/Makefile])

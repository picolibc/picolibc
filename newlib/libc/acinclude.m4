dnl For each directory which we may or may not want, we define a name
dnl for the library and an automake conditional for whether we should
dnl build the library.

LIBC_SIGNAL_LIB=
if test -n "${signal_dir}"; then
  LIBC_SIGNAL_LIB=${signal_dir}/lib.a
fi
AC_SUBST(LIBC_SIGNAL_LIB)
AM_CONDITIONAL(HAVE_SIGNAL_DIR, test x${signal_dir} != x)

LIBC_STDIO_LIB=
if test -n "${stdio_dir}"; then
  LIBC_STDIO_LIB=${stdio_dir}/lib.a
fi
AC_SUBST(LIBC_STDIO_LIB)
AM_CONDITIONAL(HAVE_STDIO_DIR, test x${stdio_dir} != x)

LIBC_STDIO64_LIB=
if test -n "${stdio64_dir}"; then
  LIBC_STDIO64_LIB=${stdio64_dir}/lib.a
fi
AC_SUBST(LIBC_STDIO64_LIB)
AM_CONDITIONAL(HAVE_STDIO64_DIR, test x${stdio64_dir} != x)

LIBC_POSIX_LIB=
if test -n "${posix_dir}"; then
  LIBC_POSIX_LIB=${posix_dir}/lib.a
fi
AC_SUBST(LIBC_POSIX_LIB)
AM_CONDITIONAL(HAVE_POSIX_DIR, test x${posix_dir} != x)

LIBC_XDR_LIB=
if test -n "${xdr_dir}"; then
  LIBC_XDR_LIB=${xdr_dir}/lib.a
fi
AC_SUBST(LIBC_XDR_LIB)
AM_CONDITIONAL(HAVE_XDR_DIR, test x${xdr_dir} != x)

LIBC_SYSCALL_LIB=
if test -n "${syscall_dir}"; then
  LIBC_SYSCALL_LIB=${syscall_dir}/lib.a
fi
AC_SUBST(LIBC_SYSCALL_LIB)
AM_CONDITIONAL(HAVE_SYSCALL_DIR, test x${syscall_dir} != x)

LIBC_UNIX_LIB=
if test -n "${unix_dir}"; then
  LIBC_UNIX_LIB=${unix_dir}/lib.a
fi
AC_SUBST(LIBC_UNIX_LIB)
AM_CONDITIONAL(HAVE_UNIX_DIR, test x${unix_dir} != x)

dnl We always recur into sys and machine, and let them decide what to
dnl do.  However, we do need to know whether they will produce a library.

LIBC_SYS_LIB=
if test -n "${sys_dir}"; then
  case ${sys_dir} in
    a29khif) AC_CONFIG_FILES([libc/sys/a29khif/Makefile]) ;;
    amdgcn) AC_CONFIG_FILES([libc/sys/amdgcn/Makefile]) ;;
    arm) AC_CONFIG_FILES([libc/sys/arm/Makefile]) ;;
    d10v) AC_CONFIG_FILES([libc/sys/d10v/Makefile]) ;;
    epiphany) AC_CONFIG_FILES([libc/sys/epiphany/Makefile]) ;;
    h8300hms) AC_CONFIG_FILES([libc/sys/h8300hms/Makefile]) ;;
    h8500hms) AC_CONFIG_FILES([libc/sys/h8500hms/Makefile]) ;;
    m88kbug) AC_CONFIG_FILES([libc/sys/m88kbug/Makefile]) ;;
    mmixware) AC_CONFIG_FILES([libc/sys/mmixware/Makefile]) ;;
    netware) AC_CONFIG_FILES([libc/sys/netware/Makefile]) ;;
    or1k) AC_CONFIG_FILES([libc/sys/or1k/Makefile]) ;;
    phoenix) AC_CONFIG_FILES([libc/sys/phoenix/Makefile]) ;;
    rdos) AC_CONFIG_FILES([libc/sys/rdos/Makefile]) ;;
    rtems) AC_CONFIG_FILES([libc/sys/rtems/Makefile]) ;;
    sh) AC_CONFIG_FILES([libc/sys/sh/Makefile]) ;;
    sysmec) AC_CONFIG_FILES([libc/sys/sysmec/Makefile]) ;;
    sysnec810) AC_CONFIG_FILES([libc/sys/sysnec810/Makefile]) ;;
    sysnecv850) AC_CONFIG_FILES([libc/sys/sysnecv850/Makefile]) ;;
    sysvi386) AC_CONFIG_FILES([libc/sys/sysvi386/Makefile]) ;;
    sysvnecv70) AC_CONFIG_FILES([libc/sys/sysvnecv70/Makefile]) ;;
    tic80) AC_CONFIG_FILES([libc/sys/tic80/Makefile]) ;;
    tirtos) AC_CONFIG_FILES([libc/sys/tirtos/Makefile]) ;;
    w65) AC_CONFIG_FILES([libc/sys/w65/Makefile]) ;;
    z8ksim) AC_CONFIG_FILES([libc/sys/z8ksim/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported sys_dir "${sys_dir}"]) ;;
  esac

  SYS_DIR=sys/${sys_dir}
  LIBC_SYS_LIB=${SYS_DIR}/lib.a
fi
AC_SUBST(SYS_DIR)
AM_CONDITIONAL(HAVE_SYS_DIR, test x${sys_dir} != x)
AC_SUBST(LIBC_SYS_LIB)

AC_TYPE_LONG_DOUBLE
AM_CONDITIONAL(HAVE_LONG_DOUBLE, test x"$ac_cv_type_long_double" = x"yes")

dnl iconv library will be compiled if --enable-newlib-iconv option is enabled
AM_CONDITIONAL(ENABLE_NEWLIB_ICONV, test x${newlib_iconv} != x)

dnl We have to include these unconditionally since machines might want to use
dnl AM_CONDITIONAL in their subdirs.
m4_include([libc/machine/nds32/acinclude.m4])
m4_include([libc/machine/powerpc/acinclude.m4])
m4_include([libc/machine/sh/acinclude.m4])
m4_include([libc/machine/spu/acinclude.m4])
m4_include([libc/sys/phoenix/acinclude.m4])

LIBC_MACHINE_LIB=
if test -n "${machine_dir}"; then
  case ${machine_dir} in
    a29k) AC_CONFIG_FILES([libc/machine/a29k/Makefile]) ;;
    aarch64) AC_CONFIG_FILES([libc/machine/aarch64/Makefile]) ;;
    amdgcn) AC_CONFIG_FILES([libc/machine/amdgcn/Makefile]) ;;
    arc) AC_CONFIG_FILES([libc/machine/arc/Makefile]) ;;
    arm) AC_CONFIG_FILES([libc/machine/arm/Makefile]) ;;
    bfin) AC_CONFIG_FILES([libc/machine/bfin/Makefile]) ;;
    cr16) AC_CONFIG_FILES([libc/machine/cr16/Makefile]) ;;
    cris) AC_CONFIG_FILES([libc/machine/cris/Makefile]) ;;
    crx) AC_CONFIG_FILES([libc/machine/crx/Makefile]) ;;
    csky) AC_CONFIG_FILES([libc/machine/csky/Makefile]) ;;
    d10v) AC_CONFIG_FILES([libc/machine/d10v/Makefile]) ;;
    d30v) AC_CONFIG_FILES([libc/machine/d30v/Makefile]) ;;
    epiphany) AC_CONFIG_FILES([libc/machine/epiphany/Makefile]) ;;
    fr30) AC_CONFIG_FILES([libc/machine/fr30/Makefile]) ;;
    frv) AC_CONFIG_FILES([libc/machine/frv/Makefile]) ;;
    ft32) AC_CONFIG_FILES([libc/machine/ft32/Makefile]) ;;
    h8300) AC_CONFIG_FILES([libc/machine/h8300/Makefile]) ;;
    h8500) AC_CONFIG_FILES([libc/machine/h8500/Makefile]) ;;
    hppa) AC_CONFIG_FILES([libc/machine/hppa/Makefile]) ;;
    i386) AC_CONFIG_FILES([libc/machine/i386/Makefile]) ;;
    i960) AC_CONFIG_FILES([libc/machine/i960/Makefile]) ;;
    iq2000) AC_CONFIG_FILES([libc/machine/iq2000/Makefile]) ;;
    lm32) AC_CONFIG_FILES([libc/machine/lm32/Makefile]) ;;
    m32c) AC_CONFIG_FILES([libc/machine/m32c/Makefile]) ;;
    m32r) AC_CONFIG_FILES([libc/machine/m32r/Makefile]) ;;
    m68hc11) AC_CONFIG_FILES([libc/machine/m68hc11/Makefile]) ;;
    m68k) AC_CONFIG_FILES([libc/machine/m68k/Makefile]) ;;
    m88k) AC_CONFIG_FILES([libc/machine/m88k/Makefile]) ;;
    mep) AC_CONFIG_FILES([libc/machine/mep/Makefile]) ;;
    microblaze) AC_CONFIG_FILES([libc/machine/microblaze/Makefile]) ;;
    mips) AC_CONFIG_FILES([libc/machine/mips/Makefile]) ;;
    riscv) AC_CONFIG_FILES([libc/machine/riscv/Makefile]) ;;
    mn10200) AC_CONFIG_FILES([libc/machine/mn10200/Makefile]) ;;
    mn10300) AC_CONFIG_FILES([libc/machine/mn10300/Makefile]) ;;
    moxie) AC_CONFIG_FILES([libc/machine/moxie/Makefile]) ;;
    msp430) AC_CONFIG_FILES([libc/machine/msp430/Makefile]) ;;
    mt) AC_CONFIG_FILES([libc/machine/mt/Makefile]) ;;
    nds32) AC_CONFIG_FILES([libc/machine/nds32/Makefile]) ;;
    necv70) AC_CONFIG_FILES([libc/machine/necv70/Makefile]) ;;
    nios2) AC_CONFIG_FILES([libc/machine/nios2/Makefile]) ;;
    nvptx) AC_CONFIG_FILES([libc/machine/nvptx/Makefile]) ;;
    or1k) AC_CONFIG_FILES([libc/machine/or1k/Makefile]) ;;
    powerpc) AC_CONFIG_FILES([libc/machine/powerpc/Makefile]) ;;
    pru) AC_CONFIG_FILES([libc/machine/pru/Makefile]) ;;
    rl78) AC_CONFIG_FILES([libc/machine/rl78/Makefile]) ;;
    rx) AC_CONFIG_FILES([libc/machine/rx/Makefile]) ;;
    sh) AC_CONFIG_FILES([libc/machine/sh/Makefile]) ;;
    sparc) AC_CONFIG_FILES([libc/machine/sparc/Makefile]) ;;
    spu) AC_CONFIG_FILES([libc/machine/spu/Makefile]) ;;
    tic4x) AC_CONFIG_FILES([libc/machine/tic4x/Makefile]) ;;
    tic6x) AC_CONFIG_FILES([libc/machine/tic6x/Makefile]) ;;
    tic80) AC_CONFIG_FILES([libc/machine/tic80/Makefile]) ;;
    v850) AC_CONFIG_FILES([libc/machine/v850/Makefile]) ;;
    visium) AC_CONFIG_FILES([libc/machine/visium/Makefile]) ;;
    w65) AC_CONFIG_FILES([libc/machine/w65/Makefile]) ;;
    x86_64) AC_CONFIG_FILES([libc/machine/x86_64/Makefile]) ;;
    xc16x) AC_CONFIG_FILES([libc/machine/xc16x/Makefile]) ;;
    xstormy16) AC_CONFIG_FILES([libc/machine/xstormy16/Makefile]) ;;
    z8k) AC_CONFIG_FILES([libc/machine/z8k/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported machine_dir "${machine_dir}"]) ;;
  esac

  LIBC_MACHINE_DIR=machine/${machine_dir}
  LIBC_MACHINE_LIB=${LIBC_MACHINE_DIR}/lib.a
fi
AM_CONDITIONAL(HAVE_LIBC_MACHINE_DIR, test "x${LIBC_MACHINE_DIR}" != x)
AC_SUBST(LIBC_MACHINE_DIR)
AC_SUBST(LIBC_MACHINE_LIB)

AM_CONDITIONAL(MACH_ADD_SETJMP, test "x$mach_add_setjmp" = "xtrue")

AC_CONFIG_FILES([libc/Makefile libc/argz/Makefile libc/ctype/Makefile libc/errno/Makefile libc/locale/Makefile libc/misc/Makefile libc/reent/Makefile libc/search/Makefile libc/stdio/Makefile libc/stdio64/Makefile libc/stdlib/Makefile libc/string/Makefile libc/time/Makefile libc/posix/Makefile libc/signal/Makefile libc/syscalls/Makefile libc/unix/Makefile libc/iconv/Makefile libc/iconv/ces/Makefile libc/iconv/ccs/Makefile libc/iconv/ccs/binary/Makefile libc/iconv/lib/Makefile libc/ssp/Makefile libc/xdr/Makefile])

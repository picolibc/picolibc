dnl This is the newlib/libc configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([sys.tex])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../..)

dnl Support --enable-newlib-io-pos-args used by libc/stdio
AC_ARG_ENABLE(newlib-io-pos-args,
[  --enable-newlib-io-pos-args enable printf-family positional arg support],
[case "${enableval}" in
  yes) newlib_io_pos_args=yes ;;
  no)  newlib_io_pos_args=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-io-pos-args option) ;;
 esac], [newlib_io_pos_args=no])dnl

dnl Support --enable-newlib-nano-malloc used by libc/stdlib
AC_ARG_ENABLE(newlib_nano_malloc,
[  --enable-newlib-nano-malloc    Use small-footprint nano-malloc implementation],
[case "${enableval}" in
   yes) newlib_nano_malloc=yes ;;
   no)  newlib_nano_malloc=no ;;
   *) AC_MSG_ERROR(bad value ${enableval} for newlib-nano-malloc) ;;
 esac],[newlib_nano_malloc=])

dnl Support --enable-newlib-nano-formatted-io used by libc/stdio
AC_ARG_ENABLE(newlib_nano_formatted_io,
[  --enable-newlib-nano-formatted-io    Use small-footprint nano-formatted-IO implementation],
[case "${enableval}" in
   yes) newlib_nano_formatted_io=yes ;;
   no)  newlib_nano_formatted_io=no ;;
   *) AC_MSG_ERROR(bad value ${enableval} for newlib-nano-formatted-io) ;;
 esac],[newlib_nano_formatted_io=no])
AM_CONDITIONAL(NEWLIB_NANO_FORMATTED_IO, test x$newlib_nano_formatted_io = xyes)

dnl Support --enable-retargetable-locking used by libc/sys
AC_ARG_ENABLE(newlib-retargetable-locking,
[  --enable-newlib-retargetable-locking    Allow locking routines to be retargeted at link time],
[case "${enableval}" in
   yes) newlib_retargetable_locking=yes ;;
   no)  newlib_retargetable_lock=no ;;
   *) AC_MSG_ERROR(bad value ${enableval} for newlib-retargetable-locking) ;;
 esac],[newlib_retargetable_locking=no])
AM_CONDITIONAL(NEWLIB_RETARGETABLE_LOCKING, test x$newlib_retargetable_locking = xyes)

NEWLIB_CONFIGURE(..)

AM_CONDITIONAL(NEWLIB_NANO_MALLOC, test x$newlib_nano_malloc = xyes)

CRT0=
if test "x${have_crt0}" = "xyes"; then
  CRT0=sys/${sys_dir}/crt0.o
fi
AC_SUBST(CRT0)

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
    a29khif) AC_CONFIG_FILES([sys/a29khif/Makefile]) ;;
    amdgcn) AC_CONFIG_FILES([sys/amdgcn/Makefile]) ;;
    arm) AC_CONFIG_FILES([sys/arm/Makefile]) ;;
    d10v) AC_CONFIG_FILES([sys/d10v/Makefile]) ;;
    decstation) AC_CONFIG_FILES([sys/decstation/Makefile]) ;;
    epiphany) AC_CONFIG_FILES([sys/epiphany/Makefile]) ;;
    h8300hms) AC_CONFIG_FILES([sys/h8300hms/Makefile]) ;;
    h8500hms) AC_CONFIG_FILES([sys/h8500hms/Makefile]) ;;
    m88kbug) AC_CONFIG_FILES([sys/m88kbug/Makefile]) ;;
    mmixware) AC_CONFIG_FILES([sys/mmixware/Makefile]) ;;
    netware) AC_CONFIG_FILES([sys/netware/Makefile]) ;;
    or1k) AC_CONFIG_FILES([sys/or1k/Makefile]) ;;
    phoenix) AC_CONFIG_FILES([sys/phoenix/Makefile]) ;;
    rdos) AC_CONFIG_FILES([sys/rdos/Makefile]) ;;
    rtems) AC_CONFIG_FILES([sys/rtems/Makefile]) ;;
    sh) AC_CONFIG_FILES([sys/sh/Makefile]) ;;
    sparc64) AC_CONFIG_FILES([sys/sparc64/Makefile]) ;;
    sun4) AC_CONFIG_FILES([sys/sun4/Makefile]) ;;
    sysmec) AC_CONFIG_FILES([sys/sysmec/Makefile]) ;;
    sysnec810) AC_CONFIG_FILES([sys/sysnec810/Makefile]) ;;
    sysnecv850) AC_CONFIG_FILES([sys/sysnecv850/Makefile]) ;;
    sysvi386) AC_CONFIG_FILES([sys/sysvi386/Makefile]) ;;
    sysvnecv70) AC_CONFIG_FILES([sys/sysvnecv70/Makefile]) ;;
    tic80) AC_CONFIG_FILES([sys/tic80/Makefile]) ;;
    tirtos) AC_CONFIG_FILES([sys/tirtos/Makefile]) ;;
    w65) AC_CONFIG_FILES([sys/w65/Makefile]) ;;
    z8ksim) AC_CONFIG_FILES([sys/z8ksim/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported sys_dir "${sys_dir}"]) ;;
  esac

  SYS_DIR=sys/${sys_dir}
  LIBC_SYS_LIB=${SYS_DIR}/lib.a
fi
AC_SUBST(SYS_DIR)
AM_CONDITIONAL(HAVE_SYS_DIR, test x${sys_dir} != x)
AC_SUBST(LIBC_SYS_LIB)
AC_SUBST(sys_dir)

AC_TYPE_LONG_DOUBLE
AM_CONDITIONAL(HAVE_LONG_DOUBLE, test x"$ac_cv_type_long_double" = x"yes")

dnl iconv library will be compiled if --enable-newlib-iconv option is enabled
AM_CONDITIONAL(ENABLE_NEWLIB_ICONV, test x${newlib_iconv} != x)

dnl We have to include these unconditionally since machines might want to use
dnl AM_CONDITIONAL in their subdirs.
m4_include([machine/nds32/acinclude.m4])
m4_include([machine/powerpc/acinclude.m4])
m4_include([machine/sh/acinclude.m4])
m4_include([machine/spu/acinclude.m4])
m4_include([sys/phoenix/acinclude.m4])

LIBC_MACHINE_LIB=
if test -n "${machine_dir}"; then
  case ${machine_dir} in
    a29k) AC_CONFIG_FILES([machine/a29k/Makefile]) ;;
    aarch64) AC_CONFIG_FILES([machine/aarch64/Makefile]) ;;
    amdgcn) AC_CONFIG_FILES([machine/amdgcn/Makefile]) ;;
    arc) AC_CONFIG_FILES([machine/arc/Makefile]) ;;
    arm) AC_CONFIG_FILES([machine/arm/Makefile]) ;;
    bfin) AC_CONFIG_FILES([machine/bfin/Makefile]) ;;
    cr16) AC_CONFIG_FILES([machine/cr16/Makefile]) ;;
    cris) AC_CONFIG_FILES([machine/cris/Makefile]) ;;
    crx) AC_CONFIG_FILES([machine/crx/Makefile]) ;;
    csky) AC_CONFIG_FILES([machine/csky/Makefile]) ;;
    d10v) AC_CONFIG_FILES([machine/d10v/Makefile]) ;;
    d30v) AC_CONFIG_FILES([machine/d30v/Makefile]) ;;
    epiphany) AC_CONFIG_FILES([machine/epiphany/Makefile]) ;;
    fr30) AC_CONFIG_FILES([machine/fr30/Makefile]) ;;
    frv) AC_CONFIG_FILES([machine/frv/Makefile]) ;;
    ft32) AC_CONFIG_FILES([machine/ft32/Makefile]) ;;
    h8300) AC_CONFIG_FILES([machine/h8300/Makefile]) ;;
    h8500) AC_CONFIG_FILES([machine/h8500/Makefile]) ;;
    hppa) AC_CONFIG_FILES([machine/hppa/Makefile]) ;;
    i386) AC_CONFIG_FILES([machine/i386/Makefile]) ;;
    i960) AC_CONFIG_FILES([machine/i960/Makefile]) ;;
    iq2000) AC_CONFIG_FILES([machine/iq2000/Makefile]) ;;
    lm32) AC_CONFIG_FILES([machine/lm32/Makefile]) ;;
    m32c) AC_CONFIG_FILES([machine/m32c/Makefile]) ;;
    m32r) AC_CONFIG_FILES([machine/m32r/Makefile]) ;;
    m68hc11) AC_CONFIG_FILES([machine/m68hc11/Makefile]) ;;
    m68k) AC_CONFIG_FILES([machine/m68k/Makefile]) ;;
    m88k) AC_CONFIG_FILES([machine/m88k/Makefile]) ;;
    mep) AC_CONFIG_FILES([machine/mep/Makefile]) ;;
    microblaze) AC_CONFIG_FILES([machine/microblaze/Makefile]) ;;
    mips) AC_CONFIG_FILES([machine/mips/Makefile]) ;;
    riscv) AC_CONFIG_FILES([machine/riscv/Makefile]) ;;
    mn10200) AC_CONFIG_FILES([machine/mn10200/Makefile]) ;;
    mn10300) AC_CONFIG_FILES([machine/mn10300/Makefile]) ;;
    moxie) AC_CONFIG_FILES([machine/moxie/Makefile]) ;;
    msp430) AC_CONFIG_FILES([machine/msp430/Makefile]) ;;
    mt) AC_CONFIG_FILES([machine/mt/Makefile]) ;;
    nds32) AC_CONFIG_FILES([machine/nds32/Makefile]) ;;
    necv70) AC_CONFIG_FILES([machine/necv70/Makefile]) ;;
    nios2) AC_CONFIG_FILES([machine/nios2/Makefile]) ;;
    nvptx) AC_CONFIG_FILES([machine/nvptx/Makefile]) ;;
    or1k) AC_CONFIG_FILES([machine/or1k/Makefile]) ;;
    powerpc) AC_CONFIG_FILES([machine/powerpc/Makefile]) ;;
    pru) AC_CONFIG_FILES([machine/pru/Makefile]) ;;
    rl78) AC_CONFIG_FILES([machine/rl78/Makefile]) ;;
    rx) AC_CONFIG_FILES([machine/rx/Makefile]) ;;
    sh) AC_CONFIG_FILES([machine/sh/Makefile]) ;;
    sparc) AC_CONFIG_FILES([machine/sparc/Makefile]) ;;
    spu) AC_CONFIG_FILES([machine/spu/Makefile]) ;;
    tic4x) AC_CONFIG_FILES([machine/tic4x/Makefile]) ;;
    tic6x) AC_CONFIG_FILES([machine/tic6x/Makefile]) ;;
    tic80) AC_CONFIG_FILES([machine/tic80/Makefile]) ;;
    v850) AC_CONFIG_FILES([machine/v850/Makefile]) ;;
    visium) AC_CONFIG_FILES([machine/visium/Makefile]) ;;
    w65) AC_CONFIG_FILES([machine/w65/Makefile]) ;;
    x86_64) AC_CONFIG_FILES([machine/x86_64/Makefile]) ;;
    xc16x) AC_CONFIG_FILES([machine/xc16x/Makefile]) ;;
    xstormy16) AC_CONFIG_FILES([machine/xstormy16/Makefile]) ;;
    z8k) AC_CONFIG_FILES([machine/z8k/Makefile]) ;;
    *) AC_MSG_ERROR([unsupported machine_dir "${machine_dir}"]) ;;
  esac

  LIBC_MACHINE_DIR=machine/${machine_dir}
  LIBC_MACHINE_LIB=${LIBC_MACHINE_DIR}/lib.a
fi
AM_CONDITIONAL(HAVE_LIBC_MACHINE_DIR, test "x${LIBC_MACHINE_DIR}" != x)
AC_SUBST(LIBC_MACHINE_DIR)
AC_SUBST(LIBC_MACHINE_LIB)
AC_SUBST(machine_dir)
AC_SUBST(shared_machine_dir)

AM_CONDITIONAL(MACH_ADD_SETJMP, test "x$mach_add_setjmp" = "xtrue")

AC_CONFIG_FILES([Makefile argz/Makefile ctype/Makefile errno/Makefile locale/Makefile misc/Makefile reent/Makefile search/Makefile stdio/Makefile stdio64/Makefile stdlib/Makefile string/Makefile time/Makefile posix/Makefile signal/Makefile syscalls/Makefile unix/Makefile iconv/Makefile iconv/ces/Makefile iconv/ccs/Makefile iconv/ccs/binary/Makefile iconv/lib/Makefile ssp/Makefile xdr/Makefile])
AC_OUTPUT

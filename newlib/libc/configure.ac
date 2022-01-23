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

dnl We have to enable libtool after NEWLIB_CONFIGURE because if we try and
dnl add it into NEWLIB_CONFIGURE, executable tests are made before the first
dnl line of the macro which fail because appropriate LDFLAGS are not set.
_LT_DECL_SED
_LT_PROG_ECHO_BACKSLASH
if test "${use_libtool}" = "yes"; then
LT_INIT([win32-dll])
fi

CRT0=
if test "x${have_crt0}" = "xyes"; then
  CRT0=sys/${sys_dir}/crt0.o
fi
AC_SUBST(CRT0)

dnl For each directory which we may or may not want, we define a name
dnl for the library and an automake conditional for whether we should
dnl build the library.

LIBC_SIGNAL_LIB=
LIBC_SIGNAL_DEF=
if test -n "${signal_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_SIGNAL_LIB=${signal_dir}/lib${signal_dir}.${aext}
  else
    LIBC_SIGNAL_LIB=${signal_dir}/lib.${aext}
  fi
  LIBC_SIGNAL_DEF=${signal_dir}/stmp-def
fi
AC_SUBST(LIBC_SIGNAL_LIB)
AC_SUBST(LIBC_SIGNAL_DEF)
AM_CONDITIONAL(HAVE_SIGNAL_DIR, test x${signal_dir} != x)

LIBC_STDIO_LIB=
LIBC_STDIO_DEF=
if test -n "${stdio_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_STDIO_LIB=${stdio_dir}/lib${stdio_dir}.${aext}
  else
    LIBC_STDIO_LIB=${stdio_dir}/lib.${aext}
  fi
  LIBC_STDIO_DEF=${stdio_dir}/stmp-def
fi
AC_SUBST(LIBC_STDIO_LIB)
AC_SUBST(LIBC_STDIO_DEF)
AM_CONDITIONAL(HAVE_STDIO_DIR, test x${stdio_dir} != x)

LIBC_STDIO64_LIB=
LIBC_STDIO64_DEF=
if test -n "${stdio64_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_STDIO64_LIB=${stdio64_dir}/lib${stdio64_dir}.${aext}
  else
    LIBC_STDIO64_LIB=${stdio64_dir}/lib.${aext}
  fi
  LIBC_STDIO64_DEF=${stdio64_dir}/stmp-def
fi
AC_SUBST(LIBC_STDIO64_LIB)
AC_SUBST(LIBC_STDIO64_DEF)
AM_CONDITIONAL(HAVE_STDIO64_DIR, test x${stdio64_dir} != x)

LIBC_POSIX_LIB=
LIBC_POSIX_DEF=
if test -n "${posix_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_POSIX_LIB=${posix_dir}/lib${posix_dir}.${aext}
  else
    LIBC_POSIX_LIB=${posix_dir}/lib.${aext}
  fi
  LIBC_POSIX_DEF=${posix_dir}/stmp-def
fi
AC_SUBST(LIBC_POSIX_LIB)
AC_SUBST(LIBC_POSIX_DEF)
AM_CONDITIONAL(HAVE_POSIX_DIR, test x${posix_dir} != x)

LIBC_XDR_LIB=
LIBC_XDR_DEF=
if test -n "${xdr_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_XDR_LIB=${xdr_dir}/lib${xdr_dir}.${aext}
  else
    LIBC_XDR_LIB=${xdr_dir}/lib.${aext}
  fi
  LIBC_XDR_DEF=${xdr_dir}/stmp-def
fi
AC_SUBST(LIBC_XDR_LIB)
AC_SUBST(LIBC_XDR_DEF)
AM_CONDITIONAL(HAVE_XDR_DIR, test x${xdr_dir} != x)

LIBC_SYSCALL_LIB=
if test -n "${syscall_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_SYSCALL_LIB=${syscall_dir}/lib${syscall_dir}.${aext}
  else
    LIBC_SYSCALL_LIB=${syscall_dir}/lib.${aext}
  fi
fi
AC_SUBST(LIBC_SYSCALL_LIB)
AM_CONDITIONAL(HAVE_SYSCALL_DIR, test x${syscall_dir} != x)

LIBC_UNIX_LIB=
if test -n "${unix_dir}"; then
  if test "${use_libtool}" = "yes"; then
    LIBC_UNIX_LIB=${unix_dir}/lib${unix_dir}.${aext}
  else
    LIBC_UNIX_LIB=${unix_dir}/lib.${aext}
  fi
fi
AC_SUBST(LIBC_UNIX_LIB)
AM_CONDITIONAL(HAVE_UNIX_DIR, test x${unix_dir} != x)

dnl We always recur into sys and machine, and let them decide what to
dnl do.  However, we do need to know whether they will produce a library.

LIBC_SYS_LIB=
if test -n "${sys_dir}"; then
  case ${sys_dir} in
    a29khif) AC_CONFIG_SUBDIRS(sys/a29khif) ;;
    amdgcn) AC_CONFIG_SUBDIRS(sys/amdgcn) ;;
    arm) AC_CONFIG_SUBDIRS(sys/arm) ;;
    d10v) AC_CONFIG_SUBDIRS(sys/d10v) ;;
    decstation) AC_CONFIG_SUBDIRS(sys/decstation) ;;
    epiphany) AC_CONFIG_SUBDIRS(sys/epiphany) ;;
    h8300hms) AC_CONFIG_SUBDIRS(sys/h8300hms) ;;
    h8500hms) AC_CONFIG_SUBDIRS(sys/h8500hms) ;;
    linux) AC_CONFIG_SUBDIRS(sys/linux) ;;
    m88kbug) AC_CONFIG_SUBDIRS(sys/m88kbug) ;;
    mmixware) AC_CONFIG_SUBDIRS(sys/mmixware) ;;
    netware) AC_CONFIG_SUBDIRS(sys/netware) ;;
    or1k) AC_CONFIG_SUBDIRS(sys/or1k) ;;
    phoenix) AC_CONFIG_SUBDIRS(sys/phoenix) ;;
    rdos) AC_CONFIG_SUBDIRS(sys/rdos) ;;
    rtems) AC_CONFIG_SUBDIRS(sys/rtems) ;;
    sh) AC_CONFIG_SUBDIRS(sys/sh) ;;
    sparc64) AC_CONFIG_SUBDIRS(sys/sparc64) ;;
    sun4) AC_CONFIG_SUBDIRS(sys/sun4) ;;
    sysmec) AC_CONFIG_SUBDIRS(sys/sysmec) ;;
    sysnec810) AC_CONFIG_SUBDIRS(sys/sysnec810) ;;
    sysnecv850) AC_CONFIG_SUBDIRS(sys/sysnecv850) ;;
    sysvi386) AC_CONFIG_SUBDIRS(sys/sysvi386) ;;
    sysvnecv70) AC_CONFIG_SUBDIRS(sys/sysvnecv70) ;;
    tic80) AC_CONFIG_SUBDIRS(sys/tic80) ;;
    tirtos) AC_CONFIG_SUBDIRS(sys/tirtos) ;;
    w65) AC_CONFIG_SUBDIRS(sys/w65) ;;
    z8ksim) AC_CONFIG_SUBDIRS(sys/z8ksim) ;;
    *) AC_MSG_ERROR([unsupported sys_dir "${sys_dir}"]) ;;
  esac

  SYS_DIR=sys/${sys_dir}
  if test "${use_libtool}" = "yes"; then
    LIBC_SYS_LIB=${SYS_DIR}/lib${sys_dir}.${aext}
  else
    LIBC_SYS_LIB=${SYS_DIR}/lib.${aext}
  fi
fi
AC_SUBST(SYS_DIR)
AM_CONDITIONAL(HAVE_SYS_DIR, test x${sys_dir} != x)
AC_SUBST(LIBC_SYS_LIB)
AC_SUBST(sys_dir)

AC_TYPE_LONG_DOUBLE
AM_CONDITIONAL(HAVE_LONG_DOUBLE, test x"$ac_cv_type_long_double" = x"yes")

dnl iconv library will be compiled if --enable-newlib-iconv option is enabled
AM_CONDITIONAL(ENABLE_NEWLIB_ICONV, test x${newlib_iconv} != x)

LIBC_MACHINE_LIB=
if test -n "${machine_dir}"; then
  case ${machine_dir} in
    a29k) AC_CONFIG_SUBDIRS(machine/a29k) ;;
    aarch64) AC_CONFIG_SUBDIRS(machine/aarch64) ;;
    amdgcn) AC_CONFIG_SUBDIRS(machine/amdgcn) ;;
    arc) AC_CONFIG_SUBDIRS(machine/arc) ;;
    arm) AC_CONFIG_SUBDIRS(machine/arm) ;;
    bfin) AC_CONFIG_SUBDIRS(machine/bfin) ;;
    cr16) AC_CONFIG_SUBDIRS(machine/cr16) ;;
    cris) AC_CONFIG_SUBDIRS(machine/cris) ;;
    crx) AC_CONFIG_SUBDIRS(machine/crx) ;;
    csky) AC_CONFIG_SUBDIRS(machine/csky) ;;
    d10v) AC_CONFIG_SUBDIRS(machine/d10v) ;;
    d30v) AC_CONFIG_SUBDIRS(machine/d30v) ;;
    epiphany) AC_CONFIG_SUBDIRS(machine/epiphany) ;;
    fr30) AC_CONFIG_SUBDIRS(machine/fr30) ;;
    frv) AC_CONFIG_SUBDIRS(machine/frv) ;;
    ft32) AC_CONFIG_SUBDIRS(machine/ft32) ;;
    h8300) AC_CONFIG_SUBDIRS(machine/h8300) ;;
    h8500) AC_CONFIG_SUBDIRS(machine/h8500) ;;
    hppa) AC_CONFIG_SUBDIRS(machine/hppa) ;;
    i386) AC_CONFIG_SUBDIRS(machine/i386) ;;
    i960) AC_CONFIG_SUBDIRS(machine/i960) ;;
    iq2000) AC_CONFIG_SUBDIRS(machine/iq2000) ;;
    lm32) AC_CONFIG_SUBDIRS(machine/lm32) ;;
    m32c) AC_CONFIG_SUBDIRS(machine/m32c) ;;
    m32r) AC_CONFIG_SUBDIRS(machine/m32r) ;;
    m68hc11) AC_CONFIG_SUBDIRS(machine/m68hc11) ;;
    m68k) AC_CONFIG_SUBDIRS(machine/m68k) ;;
    m88k) AC_CONFIG_SUBDIRS(machine/m88k) ;;
    mep) AC_CONFIG_SUBDIRS(machine/mep) ;;
    microblaze) AC_CONFIG_SUBDIRS(machine/microblaze) ;;
    mips) AC_CONFIG_SUBDIRS(machine/mips) ;;
    riscv) AC_CONFIG_SUBDIRS(machine/riscv) ;;
    mn10200) AC_CONFIG_SUBDIRS(machine/mn10200) ;;
    mn10300) AC_CONFIG_SUBDIRS(machine/mn10300) ;;
    moxie) AC_CONFIG_SUBDIRS(machine/moxie) ;;
    msp430) AC_CONFIG_SUBDIRS(machine/msp430) ;;
    mt) AC_CONFIG_SUBDIRS(machine/mt) ;;
    nds32) AC_CONFIG_SUBDIRS(machine/nds32) ;;
    necv70) AC_CONFIG_SUBDIRS(machine/necv70) ;;
    nios2) AC_CONFIG_SUBDIRS(machine/nios2) ;;
    nvptx) AC_CONFIG_SUBDIRS(machine/nvptx) ;;
    or1k) AC_CONFIG_SUBDIRS(machine/or1k) ;;
    powerpc) AC_CONFIG_SUBDIRS(machine/powerpc) ;;
    pru) AC_CONFIG_SUBDIRS(machine/pru) ;;
    rl78) AC_CONFIG_SUBDIRS(machine/rl78) ;;
    rx) AC_CONFIG_SUBDIRS(machine/rx) ;;
    sh) AC_CONFIG_SUBDIRS(machine/sh) ;;
    sparc) AC_CONFIG_SUBDIRS(machine/sparc) ;;
    spu) AC_CONFIG_SUBDIRS(machine/spu) ;;
    tic4x) AC_CONFIG_SUBDIRS(machine/tic4x) ;;
    tic6x) AC_CONFIG_SUBDIRS(machine/tic6x) ;;
    tic80) AC_CONFIG_SUBDIRS(machine/tic80) ;;
    v850) AC_CONFIG_SUBDIRS(machine/v850) ;;
    visium) AC_CONFIG_SUBDIRS(machine/visium) ;;
    w65) AC_CONFIG_SUBDIRS(machine/w65) ;;
    x86_64) AC_CONFIG_SUBDIRS(machine/x86_64) ;;
    xc16x) AC_CONFIG_SUBDIRS(machine/xc16x) ;;
    xstormy16) AC_CONFIG_SUBDIRS(machine/xstormy16) ;;
    z8k) AC_CONFIG_SUBDIRS(machine/z8k) ;;
    *) AC_MSG_ERROR([unsupported machine_dir "${machine_dir}"]) ;;
  esac

  LIBC_MACHINE_DIR=machine/${machine_dir}
  if test "${use_libtool}" = "yes"; then
    LIBC_MACHINE_LIB=${LIBC_MACHINE_DIR}/lib${machine_dir}.${aext}
  else
    LIBC_MACHINE_LIB=${LIBC_MACHINE_DIR}/lib.${aext}
  fi
fi
AM_CONDITIONAL(HAVE_LIBC_MACHINE_DIR, test "x${LIBC_MACHINE_DIR}" != x)
AC_SUBST(LIBC_MACHINE_DIR)
AC_SUBST(LIBC_MACHINE_LIB)
AC_SUBST(machine_dir)
AC_SUBST(shared_machine_dir)

AC_CONFIG_FILES([Makefile argz/Makefile ctype/Makefile errno/Makefile locale/Makefile misc/Makefile reent/Makefile search/Makefile stdio/Makefile stdio64/Makefile stdlib/Makefile string/Makefile time/Makefile posix/Makefile signal/Makefile syscalls/Makefile unix/Makefile iconv/Makefile iconv/ces/Makefile iconv/ccs/Makefile iconv/ccs/binary/Makefile iconv/lib/Makefile ssp/Makefile xdr/Makefile])
AC_OUTPUT

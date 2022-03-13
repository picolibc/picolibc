dnl For each directory which we may or may not want, we define a name
dnl for the library and an automake conditional for whether we should
dnl build the library.

AM_CONDITIONAL(HAVE_SIGNAL_DIR, test x${signal_dir} != x)
AM_CONDITIONAL(HAVE_STDIO_DIR, test x${stdio_dir} != x)
AM_CONDITIONAL(HAVE_STDIO64_DIR, test x${stdio64_dir} != x)
AM_CONDITIONAL(HAVE_POSIX_DIR, test x${posix_dir} != x)
AM_CONDITIONAL(HAVE_XDR_DIR, test x${xdr_dir} != x)
AM_CONDITIONAL(HAVE_SYSCALL_DIR, test x${syscall_dir} != x)
AM_CONDITIONAL(HAVE_UNIX_DIR, test x${unix_dir} != x)

dnl We always recur into sys and machine, and let them decide what to do.
m4_foreach_w([SYS_DIR], [
  a29khif amdgcn arm
  d10v
  epiphany
  h8300hms h8500hms
  m88kbug mmixware
  netware
  or1k
  rdos rtems
  sh sysmec sysnec810 sysnecv850 sysvi386 sysvnecv70
  tic80 tirtos
  w65
  z8ksim
], [AM_CONDITIONAL([HAVE_LIBC_SYS_]m4_toupper(SYS_DIR)[_DIR], test "${sys_dir}" = SYS_DIR)])

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

m4_foreach_w([MACHINE], [
  aarch64 amdgcn arc arm
  bfin
  cr16 cris crx csky
  d10v d30v
  epiphany
  fr30 frv ft32
  h8300 h8500 hppa
  i386 i960 iq2000
  lm32
  m32c m32r m68hc11 m68k m88k mep microblaze mips mn10200 mn10300 moxie msp430 mt
  nds32 necv70 nios2 nvptx
  or1k
  powerpc pru
  riscv rl78 rx
  sh sparc spu
  tic4x tic6x tic80
  v850 visium
  w65
  x86_64 xc16x xstormy16
  z8k
], [AM_CONDITIONAL([HAVE_LIBC_MACHINE_]m4_toupper(MACHINE), test "${machine_dir}" = MACHINE)])

AM_CONDITIONAL(MACH_ADD_SETJMP, test "x$mach_add_setjmp" = "xtrue")

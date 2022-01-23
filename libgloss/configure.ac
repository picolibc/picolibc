dnl Process this file with autoconf to produce a configure script.
AC_INIT([libgloss],[LIBGLOSS_VERSION])
AC_CONFIG_SRCDIR([libnosys])

if test "${enable_shared}" = "yes" ; then
  AC_MSG_WARN([Shared libraries not supported for cross compiling, ignored])
fi

AC_CONFIG_AUX_DIR(..)

AC_PROG_INSTALL

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign no-define 1.15])

if test -z "${with_multisubdir}" ; then
  AC_CONFIG_FILES([doc/Makefile])
fi

dnl indicates whether to run configure within the testsuite subdirectory
config_testsuite=true

dnl indicates whether to run configure within the libnosys subdirectory
config_libnosys=true

case "${target}" in
  aarch64*-*-*)
	AC_CONFIG_SUBDIRS(aarch64)
	config_testsuite=true
	;;
  arc*-*-*)
	AC_CONFIG_FILES([arc/Makefile])
	subdirs="$subdirs arc"
	;;
  csky*-*-*)
	AC_CONFIG_SUBDIRS(csky)
	;;
  epiphany-*-*)
	AC_CONFIG_SUBDIRS(epiphany)
	config_testsuite=true
	;;
  i[[3456]]86-*-elf* | i[[3456]]86-*-coff*)
	AC_CONFIG_SUBDIRS([i386])
	;;
  m32r-*-*)
	AC_CONFIG_FILES([m32r/Makefile])
	subdirs="$subdirs m32r"
	;;
  m68hc11-*-* | m6811-*-* | m68hc12-*-* | m6812-*-*)
	AC_CONFIG_FILES([m68hc11/Makefile])
	subdirs="$subdirs m68hc11"
	;;
  nds32*-*-*)
	AC_CONFIG_FILES([nds32/Makefile])
	subdirs="$subdirs nds32"
	;;
  fido-*-* | m68*-*-*)
	AC_CONFIG_SUBDIRS([m68k])
	;;
  hppa*-*-pro*)
	AC_CONFIG_FILES([pa/Makefile])
	subdirs="$subdirs pa"
	;;
  i960-*-coff)
	AC_CONFIG_FILES([i960/Makefile])
	subdirs="$subdirs i960"
	;;
  sparc-*leon*-elf* | sparc-*leon*-none*)
	AC_CONFIG_FILES([sparc_leon/Makefile])
	subdirs="$subdirs sparc_leon"
	;;
  sparclet-*-aout* | sparc-*-elf* | sparc64-*-elf* | sparc86x-*-* | sparclite-*-*)
	AC_CONFIG_SUBDIRS([sparc])
	;;
  sh*-*-pe | mips*-*-pe | *arm*-wince-pe)
	AC_CONFIG_SUBDIRS([wince])
	;;
  mips*-*-*)
	AC_CONFIG_SUBDIRS([mips])
	;;
  riscv*-*-*)
	AC_CONFIG_FILES([riscv/Makefile])
	subdirs="$subdirs riscv"
	;;
  powerpc-*-*|powerpcle-*-*)
	AC_CONFIG_SUBDIRS([rs6000])
	;;
  mn10200-*-*)
	AC_CONFIG_SUBDIRS([mn10200])
	;;
  mn10300-*-*)
	AC_CONFIG_SUBDIRS([mn10300])
	;;
  bfin-*-*)
	AC_CONFIG_FILES([bfin/Makefile])
	subdirs="$subdirs bfin"
	;;
  cr16-*-*)
	AC_CONFIG_FILES([cr16/Makefile])
	subdirs="$subdirs cr16"
	;;
  cris-*-* | crisv32-*-*)
	AC_CONFIG_FILES([cris/Makefile])
	subdirs="$subdirs cris"
	;;
  crx-*-*)
	AC_CONFIG_FILES([crx/Makefile])
	subdirs="$subdirs crx"
	;;
  d30v-*-*)
	AC_CONFIG_FILES([d30v/Makefile])
	subdirs="$subdirs d30v"
	;;
  fr30-*-*)
	AC_CONFIG_FILES([fr30/Makefile])
	subdirs="$subdirs fr30"
	;;
  frv*-*-*)
	AC_CONFIG_FILES([frv/Makefile])
	subdirs="$subdirs frv"
	;;
  ft32*-*-*)
	AC_CONFIG_FILES([ft32/Makefile])
	subdirs="$subdirs ft32"
	;;
  lm32*-*-*)
	AC_CONFIG_FILES([lm32/Makefile])
	subdirs="$subdirs lm32"
	;;
  mcore-*-*)
	AC_CONFIG_SUBDIRS([mcore])
	;;
  mep-*-*)
	AC_CONFIG_FILES([mep/Makefile])
	subdirs="$subdirs mep"
	;;
  moxie-*-*)
	AC_CONFIG_SUBDIRS([moxie])
	;;
  microblaze*-*-*)
	AC_CONFIG_FILES([microblaze/Makefile])
	subdirs="$subdirs microblaze"
	;;
  mt-*-*)
	AC_CONFIG_FILES([mt/Makefile])
	subdirs="$subdirs mt"
	;;
  v850*-*-*)
	AC_CONFIG_FILES([v850/Makefile])
	subdirs="$subdirs v850"
	;;
  visium-*-*)
	AC_CONFIG_FILES([visium/Makefile])
	subdirs="$subdirs visium"
	config_testsuite=false
	;;
  xc16x-*-*)
	AC_CONFIG_SUBDIRS([xc16x])
	;;
  xstormy16-*-*)
	AC_CONFIG_FILES([xstormy16/Makefile])
	subdirs="$subdirs xstormy16"
	;;
  m32c-*-*)
	AC_CONFIG_FILES([m32c/Makefile])
	subdirs="$subdirs m32c"
	;;
  msp430-*-elf*)
	AC_CONFIG_FILES([msp430/Makefile])
	subdirs="$subdirs msp430"
	config_libnosys=false
	;;
  rl78*-*-elf)
	AC_CONFIG_FILES([rl78/Makefile])
	subdirs="$subdirs rl78"
	;;
  rx*-*-elf)
	AC_CONFIG_FILES([rx/Makefile])
	subdirs="$subdirs rx"
	;;
  arm*-*-elf | arm*-*-coff | arm*-*-*)
	AC_CONFIG_SUBDIRS([arm])
	;;
  spu-*-elf)
	AC_CONFIG_SUBDIRS([spu])
	config_testsuite=false
	config_libnosys=false
	;;
  tic6x-*-*)
	AC_CONFIG_FILES([tic6x/Makefile])
	subdirs="$subdirs tic6x"
	config_testsuite=false
	;;
  iq2000-*-*)
	AC_CONFIG_SUBDIRS([iq2000])
	;;
  or1k-*-* | or1knd-*-* )
	AC_CONFIG_SUBDIRS([or1k])
	;;
  pru-*-*)
	AC_CONFIG_FILES([pru/Makefile])
	subdirs="$subdirs pru"
	;;
  nios2-*-*)
	AC_CONFIG_FILES([nios2/Makefile])
	subdirs="$subdirs nios2"
	;;
esac

dnl For now, don't bother configuring testsuite
dnl
dnl if test "${config_testsuite}" = "true";
dnl   then AC_CONFIG_SUBDIRS([testsuite])
dnl fi

if test "${config_libnosys}" = "true"; then
  AC_CONFIG_SUBDIRS([libnosys])
fi

LIB_AC_PROG_CC
AS=${AS-as}
AC_SUBST(AS)
AC_CHECK_PROG(AR, ar, ar, :)
AR=${AR-ar}
AC_SUBST(AR)
LD=${LD-ld}
AC_SUBST(LD)
AC_PROG_RANLIB
LIB_AM_PROG_AS

host_makefile_frag=${srcdir}/config/default.mh
target_makefile_frag=${srcdir}/config/default.mt

dnl We have to assign the same value to other variables because autoconf
dnl doesn't provide a mechanism to substitute a replacement keyword with
dnl arbitrary data or pathnames.
dnl
host_makefile_frag_path=$host_makefile_frag
AC_SUBST(host_makefile_frag_path)
AC_SUBST_FILE(host_makefile_frag)
target_makefile_frag_path=$target_makefile_frag
AC_SUBST(target_makefile_frag_path)
AC_SUBST_FILE(target_makefile_frag)

if test "${multilib}" = "yes"; then
  multilib_arg="--enable-multilib"
else
  multilib_arg=
fi

# for now, only add multilibs for specific targets
AC_CONFIG_FILES([Makefile],
[if test -n "$CONFIG_FILES"; then
  unset ac_file
  . ${srcdir}/../config-ml.in
fi],
srcdir=${srcdir}
target=${target}
host=${host}
with_multisubdir=${with_multisubdir}
ac_configure_args="--enable-multilib ${ac_configure_args}"
CONFIG_SHELL=${CONFIG_SHELL-/bin/sh}
CC="${CC}"
LDFLAGS=${LDFLAGS}
)
AC_OUTPUT



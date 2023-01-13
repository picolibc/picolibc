dnl Process this file with autoconf to produce a configure script.
AC_INIT([libgloss],[LIBGLOSS_VERSION])
AC_CONFIG_HEADER(config.h)

if test "${enable_shared}" = "yes" ; then
  AC_MSG_WARN([Shared libraries not supported for cross compiling, ignored])
fi

AC_CONFIG_AUX_DIR(..)

dnl Support --disable-newlib-supplied-syscalls
dnl NB: This is kept in sync with the newlib/acinclude.m4 option code.
AC_ARG_ENABLE(newlib-supplied-syscalls,
[  --disable-newlib-supplied-syscalls disable newlib from supplying syscalls],
[case "${enableval}" in
  yes) newlib_may_supply_syscalls=yes ;;
  no)  newlib_may_supply_syscalls=no ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for newlib-supplied-syscalls option) ;;
 esac], [newlib_may_supply_syscalls=yes])dnl

AM_CONDITIONAL(MAY_SUPPLY_SYSCALLS, test x[$]{newlib_may_supply_syscalls} = xyes)

AC_PROG_INSTALL

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign no-dist no-define subdir-objects 1.15.1])
AM_MAINTAINER_MODE
AM_SILENT_RULES(yes)

if test -z "${with_multisubdir}"; then
  have_multisubdir=no
else
  have_multisubdir=yes
fi
AM_CONDITIONAL(HAVE_DOC, test x$have_multisubdir = xno)
AM_CONDITIONAL(HAVE_MULTISUBDIR, test x$have_multisubdir = xyes)

AC_NO_EXECUTABLES

dnl indicates whether to run configure within the testsuite subdirectory
config_testsuite=true

dnl indicates whether to run configure within the libnosys subdirectory
config_libnosys=true

host_makefile_frag=${srcdir}/config/default.mh
target_makefile_frag=${srcdir}/config/default.mt

case "${target}" in
  aarch64*-*-*)
	config_aarch64=true
	config_testsuite=true
	;;
  arc*-*-*)
	config_arc=true
	;;
  csky*-*-*)
	config_csky=true
	;;
  epiphany-*-*)
	AC_CONFIG_FILES([epiphany/Makefile])
	subdirs="$subdirs epiphany"
	config_testsuite=true
	;;
  i[[3456]]86-*-elf* | i[[3456]]86-*-coff*)
	AC_CONFIG_FILES([i386/Makefile])
	subdirs="$subdirs i386"
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
	AC_CONFIG_FILES([m68k/Makefile])
	subdirs="$subdirs m68k"
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
	AC_CONFIG_FILES([sparc/Makefile])
	subdirs="$subdirs sparc"
	;;
  sh*-*-pe | mips*-*-pe | *arm*-wince-pe)
	config_wince=true
	;;
  mips*-*-*)
	AC_CONFIG_FILES([mips/Makefile])
	subdirs="$subdirs mips"
	target_makefile_frag=${srcdir}/config/mips.mt
	;;
  riscv*-*-*)
	config_riscv=true
	;;
  powerpc-*-*|powerpcle-*-*)
	AC_CONFIG_FILES([rs6000/Makefile])
	subdirs="$subdirs rs6000"
	host_makefile_frag=${srcdir}/config/ppc.mh
	;;
  mn10200-*-*)
	AC_CONFIG_FILES([mn10200/Makefile])
	subdirs="$subdirs mn10200"
	target_makefile_frag=${srcdir}/config/mn10200.mt
	;;
  mn10300-*-*)
	AC_CONFIG_FILES([mn10300/Makefile])
	subdirs="$subdirs mn10300"
	target_makefile_frag=${srcdir}/config/mn10300.mt
	;;
  bfin-*-*)
	config_bfin=true
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
	config_d30v=true
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
	config_lm32=true
	;;
  mcore-*-*)
	AC_CONFIG_FILES([mcore/Makefile])
	subdirs="$subdirs mcore"
	;;
  mep-*-*)
	AC_CONFIG_FILES([mep/Makefile])
	subdirs="$subdirs mep"
	;;
  moxie-*-*)
	AC_CONFIG_FILES([moxie/Makefile])
	subdirs="$subdirs moxie"
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
	AC_CONFIG_FILES([xc16x/Makefile])
	subdirs="$subdirs xc16x"
	target_makefile_frag=${srcdir}/config/xc16x.mt
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
	config_arm=true
	;;
  spu-*-elf)
	AC_CONFIG_FILES([spu/Makefile])
	subdirs="$subdirs spu"
	host_makefile_frag=${srcdir}/config/ppc.mh
	config_testsuite=false
	config_libnosys=false
	;;
  tic6x-*-*)
	AC_CONFIG_FILES([tic6x/Makefile])
	subdirs="$subdirs tic6x"
	config_testsuite=false
	;;
  iq2000-*-*)
	config_iq2000=true
	;;
  or1k-*-* | or1knd-*-* )
	AC_CONFIG_FILES([or1k/Makefile])
	subdirs="$subdirs or1k"
	;;
  pru-*-*)
	AC_CONFIG_FILES([pru/Makefile])
	subdirs="$subdirs pru"
	;;
  nios2-*-*)
	config_nios2=true
	;;
esac
AC_SUBST(subdirs)

dnl These subdirs have converted to non-recursive make.  Hopefully someday all
dnl the ports above will too!
m4_foreach_w([SUBDIR], [
  aarch64 arc arm bfin csky d30v iq2000 libnosys lm32 nios2 riscv wince
], [dnl
  AM_CONDITIONAL([CONFIG_]m4_toupper(SUBDIR), [test x$config_]SUBDIR = xtrue)
])

dnl For now, don't bother configuring testsuite
dnl
dnl if test "${config_testsuite}" = "true";
dnl   then AC_CONFIG_SUBDIRS([testsuite])
dnl fi

AC_PROG_CC
AM_PROG_AS
AM_PROG_AR
LD=${LD-ld}
AC_SUBST(LD)
AC_PROG_RANLIB

AC_DEFINE(HAVE_GNU_LD, 1, [Using GNU LD])

AC_CACHE_CHECK([if target is ELF], libc_cv_is_elf, [dnl
  AC_PREPROC_IFELSE([AC_LANG_PROGRAM(
[[#ifndef __ELF__
# error "not ELF"
#endif
]])], [libc_cv_is_elf=yes], [libc_cv_is_elf=no])
])
if test "x${libc_cv_is_elf}" = "xyes"; then
  AC_DEFINE(HAVE_ELF, 1, [Using ELF format])
fi

AC_CACHE_CHECK([for .previous assembler directive],
	       libc_cv_asm_previous_directive, [dnl
  libc_cv_asm_previous_directive=no
  cat > conftest.s <<EOF
.section foo_section
.previous
EOF
  if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s 1>&AC_FD_CC); then
    libc_cv_asm_previous_directive=yes
  fi
  rm -f conftest*])
if test "x${libc_cv_asm_previous_directive}" = "xyes"; then
  AC_DEFINE(HAVE_ASM_PREVIOUS_DIRECTIVE, 1, [.previous directive allowed])
fi

AC_CACHE_CHECK([for .popsection assembler directive],
	       libc_cv_asm_popsection_directive, [dnl
  libc_cv_asm_popsection_directive=no
  cat > conftest.s <<EOF
.pushsection foo_section
.popsection
EOF
  if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s 1>&AC_FD_CC); then
    libc_cv_asm_popsection_directive=yes
  fi
  rm -f conftest*])
if test "x${libc_cv_asm_popsection_directive}" = "xyes"; then
  AC_DEFINE(HAVE_ASM_POPSECTION_DIRECTIVE, 1, [.pushsection/.popsection directives allowed])
fi

AC_CACHE_CHECK([for section attributes],
	       libc_cv_section_attributes, [dnl
  libc_cv_section_attributes=no
  cat > conftest.c <<EOF
int secttest __attribute__ ((section (".gnu.warning.secttest"))) = 10;
int main() {}
EOF
  if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.c 1>&AC_FD_CC); then
    libc_cv_section_attributes=yes
  fi
  rm -f conftest*])
if test "x${libc_cv_section_attributes}" = "xyes"; then
  AC_DEFINE(HAVE_SECTION_ATTRIBUTES, 1, [support for section attributes])
fi

AC_CACHE_CHECK([for symbol prefix], libc_cv_symbol_prefix, [dnl
  cat > conftest.c <<\EOF
foo () { }
EOF
dnl
  libc_cv_symbol_prefix=''
  if AC_TRY_COMMAND([${CC-cc} -S conftest.c -o - | fgrep "\$foo" > /dev/null]); then
    libc_cv_symbol_prefix='$'
  else
    if AC_TRY_COMMAND([${CC-cc} -S conftest.c -o - | fgrep "_foo" > /dev/null]); then
      libc_cv_symbol_prefix=_
    fi
  fi
  rm -f conftest* ])
AC_DEFINE_UNQUOTED(__SYMBOL_PREFIX, "$libc_cv_symbol_prefix", [symbol prefix])

m4_include([aarch64/acinclude.m4])
m4_include([arm/acinclude.m4])
m4_include([i386/acinclude.m4])
m4_include([libnosys/acinclude.m4])
m4_include([m68k/acinclude.m4])
m4_include([mcore/acinclude.m4])
m4_include([mips/acinclude.m4])
m4_include([mn10300/acinclude.m4])
m4_include([moxie/acinclude.m4])

AC_CACHE_CHECK([for compiler search settings], libc_cv_compiler_isystem, [dnl
  for subdir in include include-fixed; do
    AS_IF([dir=`$CC -print-file-name=$subdir`], [dnl
      AS_IF([test -n "$dir"], [dnl
        libc_cv_compiler_isystem="$libc_cv_compiler_isystem -isystem $dir"
      ])
    ])
  done
])
CC="$CC -nostdinc $libc_cv_compiler_isystem"

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

dnl AM_ENABLE_MULTILIB only processes the top-level Makefile, but we want the
dnl multilib settings to propagate to all sub-Makefiles that we recurse into
dnl too.  This can go away if we ever get rid of sub-Makefiles and only use
dnl Automake + makefile include fragments.
AC_OUTPUT_COMMANDS([
for subdir in $subdirs; do
  case " $CONFIG_FILES " in
   *" $subdir/Makefile "*)
     ac_file=$subdir/Makefile . ${multi_basedir}/config-ml.in
     ;;
  esac
done], [subdirs="$subdirs"])

AM_ENABLE_MULTILIB(, ..)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

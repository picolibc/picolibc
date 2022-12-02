dnl We have to include these unconditionally since machines might want to use
dnl AM_CONDITIONAL in their subdirs.
m4_include([libm/machine/nds32/acinclude.m4])

dnl Define HAVE_LIBM_MACHINE_<machine> automake conditionals.
m4_foreach_w([MACHINE], [
  aarch64 amdgcn arm i386 mips nds32 powerpc pru sparc spu riscv x86_64
], [dnl
  AM_CONDITIONAL([HAVE_LIBM_MACHINE_]m4_toupper(MACHINE), test "${libm_machine_dir}" = "MACHINE")
])

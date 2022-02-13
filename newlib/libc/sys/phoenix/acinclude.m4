AS_IF([test "$sys_dir" = "phoenix"], [dnl
  PHOENIX_MACHINE_DIR=machine/${machine_dir}
])
AC_SUBST(PHOENIX_MACHINE_DIR)

m4_foreach_w([MACHINE], [
  arm
], [AM_CONDITIONAL([HAVE_LIBC_SYS_PHOENIX_]m4_toupper(MACHINE)[_DIR], test "${machine_dir}" = MACHINE)])

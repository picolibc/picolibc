AS_IF([test "$sys_dir" = "phoenix"], [dnl
  m4_foreach_w([MACHINE], [
    arm
  ], [dnl
    AS_IF(test "${machine_dir}" = MACHINE, AC_CONFIG_FILES([sys/phoenix/machine/]MACHINE[/Makefile]))
  ])

  PHOENIX_MACHINE_DIR=machine/${machine_dir}
])
AC_SUBST(PHOENIX_MACHINE_DIR)

if test "x$newlib_may_supply_syscalls" = "xyes"; then
  ARM_BUILD_CRT0_TRUE='#'
  ARM_BUILD_CRT0_FALSE=
else
  ARM_BUILD_CRT0_TRUE=
  ARM_BUILD_CRT0_FALSE='#'
fi
AC_SUBST(ARM_BUILD_CRT0_TRUE)
AC_SUBST(ARM_BUILD_CRT0_FALSE)

ARM_OBJTYPE=
case "${target}" in
  *-*-elf | *-*-eabi* | *-*-tirtos*)
	ARM_OBJTYPE=elf-
	;;
  *-*-coff)
	ARM_OBJTYPE=coff-
	;;
esac
AC_SUBST(ARM_OBJTYPE)

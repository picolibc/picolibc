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

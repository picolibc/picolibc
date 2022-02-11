MCORE_BSP_PREFIX=
case "${target}" in
  mcore-*-elf)
	MCORE_BSP_PREFIX=elf-
	;;
  mcore-*-pe)
	MCORE_BSP_PREFIX=pe-
	;;
esac
AC_SUBST(MCORE_BSP_PREFIX)

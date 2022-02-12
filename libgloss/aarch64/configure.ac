dnl Process this file with autoconf to produce a configure script.
AC_INIT(crt0.S)

AC_CONFIG_AUX_DIR(../..)

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AC_PROG_INSTALL

AC_NO_EXECUTABLES
AC_PROG_CC
AM_PROG_AS
AM_PROG_AR
LD=${LD-ld}
AC_SUBST(LD)
AC_PROG_RANLIB

case "${target}" in
  *-*-elf)
	objtype=elf-
	;;
esac

AC_SUBST(objtype)

host_makefile_frag=`cd $srcdir/../config;pwd`/default.mh

dnl We have to assign the same value to other variables because autoconf
dnl doesn't provide a mechanism to substitute a replacement keyword with
dnl arbitrary data or pathnames.
dnl
host_makefile_frag_path=$host_makefile_frag
AC_SUBST(host_makefile_frag_path)
AC_SUBST_FILE(host_makefile_frag)

# Configure cpu init plug-ins
if test -d "${srcdir}/cpu-init"; then
  subdirs="${subdirs} cpu-init"
  AC_CONFIG_FILES([cpu-init/Makefile], [dnl
    ac_file=cpu-init/Makefile . ${srcdir}/../../config-ml.in
  ])
  AC_SUBST(subdirs)
fi

AM_ENABLE_MULTILIB(, ../..)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

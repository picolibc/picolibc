dnl This is the newlib/libc/machine/powerpc configure.in file.
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([newlib],[NEWLIB_VERSION])
AC_CONFIG_SRCDIR([setjmp.S])

dnl Can't be done in NEWLIB_CONFIGURE because that confuses automake. 
AC_CONFIG_AUX_DIR(../../../..)

NEWLIB_CONFIGURE(../../..)

extra_objs=
case $host in
  powerpc*-*altivec*)
	extra_objs="lib_a-vfprintf.o lib_a-vfscanf.o lib_a-vec_malloc.o lib_a-vec_calloc.o lib_a-vec_free.o lib_a-vec_realloc.o lib_a-vec_mallocr.o lib_a-vec_callocr.o lib_a-vec_reallocr.o"
	;;
  powerpc*-*spe*)
	extra_objs="lib_a-atosfix16.o lib_a-atosfix32.o lib_a-atosfix64.o lib_a-atoufix16.o lib_a-atoufix32.o lib_a-atoufix64.o lib_a-simdldtoa.o lib_a-strtosfix16.o lib_a-strtosfix32.o lib_a-strtosfix64.o lib_a-strtoufix16.o lib_a-strtoufix32.o lib_a-strtoufix64.o lib_a-ufix64toa.o lib_a-vfprintf.o lib_a-vfscanf.o"
	;;
esac
AC_SUBST(extra_objs)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

#=========================================================================
# configure.ac for riscv libgloss and crt0
#=========================================================================

#-------------------------------------------------------------------------
# Setup
#-------------------------------------------------------------------------

AC_INIT([crt0.S])
AC_CONFIG_SRCDIR([crt0.S])
AC_CONFIG_AUX_DIR(../..)

#-------------------------------------------------------------------------
# Checks for programs
#-------------------------------------------------------------------------

LIB_AC_PROG_CC
AC_CHECK_TOOL([AR],[ar])
AC_CHECK_TOOL([RANLIB],[ranlib])
AC_PROG_INSTALL

#-------------------------------------------------------------------------
# Output
#-------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile,
. ${srcdir}/../../config-ml.in,
srcdir=${srcdir}
target=${target}
with_multisubdir=${with_multisubdir}
ac_configure_args="${ac_configure_args} --enable-multilib"
CONFIG_SHELL=${CONFIG_SHELL-/bin/sh}
)
AC_OUTPUT

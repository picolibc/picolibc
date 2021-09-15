# Makefile for libgloss/epiphany

# Copyright (c) 2011, Adapteva, Inc.
# All rights reserved.

# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com> for Adapteva Inc

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of Adapteva nor the names of its contributors may be
#    used to endorse or promote products derived from this software without
#    specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.                                               */

# Process this file with autoconf to produce a configure script

AC_PREREQ(2.59)
AC_INIT(libepiphany,0.0.1)
AC_CONFIG_HEADER(config.h)

# No shared libraries allowed
if test "${enable_shared}" = "yes" ; then
    echo "Shared libraries not supported for cross compiling, ignored"
fi

# Where are the auxillary tools (confg.sub etc)?
if test "$srcdir" = "." ; then
  if test "${with_target_subdir}" != "." ; then
    libgloss_topdir="${srcdir}/${with_multisrctop}../../.."
  else
    libgloss_topdir="${srcdir}/${with_multisrctop}../.."
  fi
else
  libgloss_topdir="${srcdir}/../.."
fi
AC_CONFIG_AUX_DIR($libgloss_topdir)

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AC_PROG_INSTALL

AC_DEFINE(HAVE_GNU_LD, 1, [Using GNU ld])

# We always use ELF, define various useful associated things.
AC_DEFINE(HAVE_ELF, 1, [Using ELF format])

# Assembler directives (the assembler handles them, whether it does anything
# useful with them is another matter...
AC_DEFINE(HAVE_ASM_PREVIOUS_DIRECTIVE, 1, [.previous directive allowed])
AC_DEFINE(HAVE_ASM_POPSECTION_DIRECTIVE, 1,
          [.pushsection/.popsection directives allowed])

# Section attributes in C don't currently work
#AC_DEFINE(HAVE_SECTION_ATTRIBUTES, 1, [support for section attributes])

# Sort out what the symbol prefix is (we could just fix it as '_', but let the
# script work it out.
AC_CACHE_CHECK([for symbol prefix], libc_cv_symbol_prefix, [dnl
cat > conftest.c <<\EOF
foo () { }
EOF
dnl
libc_cv_symbol_prefix=none
if AC_TRY_COMMAND([${CC-cc} -S conftest.c -o - | fgrep "\$foo" > /dev/null]);
then
  libc_cv_symbol_prefix='$'
else
  if AC_TRY_COMMAND([${CC-cc} -S conftest.c -o - | fgrep "_foo" > /dev/null]);
  then
    libc_cv_symbol_prefix=_
  fi
fi
rm -f conftest* ])
if test $libc_cv_symbol_prefix != none; then
  AC_DEFINE_UNQUOTED(__SYMBOL_PREFIX, "$libc_cv_symbol_prefix", [symbol prefix])
else
  AC_DEFINE(__SYMBOL_PREFIX, "", [symbol prefix])
fi

# Standard stuff copied from libnosys. For this we'll need to an aclocal.m4
LIB_AC_PROG_CC
AS=${AS-as}
AC_SUBST(AS)
AR=${AR-ar}
AC_SUBST(AR)
LD=${LD-ld}
AC_SUBST(LD)
AC_PROG_RANLIB
LIB_AM_PROG_AS

host_makefile_frag=${srcdir}/../config/default.mh

dnl We have to assign the same value to other variables because autoconf
dnl doesn't provide a mechanism to substitute a replacement keyword with
dnl arbitrary data or pathnames.
dnl
host_makefile_frag_path=$host_makefile_frag
AC_SUBST(host_makefile_frag_path)
AC_SUBST_FILE(host_makefile_frag)

AC_CONFIG_FILES(Makefile,
ac_file=Makefile . ${libgloss_topdir}/config-ml.in,
srcdir=${srcdir}
target=${target}
with_multisubdir=${with_multisubdir}
ac_configure_args="${ac_configure_args} --enable-multilib"
CONFIG_SHELL=${CONFIG_SHELL-/bin/sh}
libgloss_topdir=${libgloss_topdir}
)
AC_OUTPUT

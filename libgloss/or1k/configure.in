# Makefile for libgloss/or1k

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
AC_INIT(libor1k,0.0.1)

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
target_makefile_frag=${srcdir}/../config/default.mt

dnl We have to assign the same value to other variables because autoconf
dnl doesn't provide a mechanism to substitute a replacement keyword with
dnl arbitrary data or pathnames.
dnl
host_makefile_frag_path=$host_makefile_frag
AC_SUBST(host_makefile_frag_path)
AC_SUBST_FILE(host_makefile_frag)
target_makefile_frag_path=$target_makefile_frag
AC_SUBST(target_makefile_frag_path)
AC_SUBST_FILE(target_makefile_frag)

AC_CONFIG_FILES(Makefile,
. ${libgloss_topdir}/config-ml.in,
srcdir=${srcdir}
target=${target}
with_multisubdir=${with_multisubdir}
ac_configure_args="${ac_configure_args} --enable-multilib"
CONFIG_SHELL=${CONFIG_SHELL-/bin/sh}
libgloss_topdir=${libgloss_topdir}
)
AC_OUTPUT
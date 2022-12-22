#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2022 Keith Packard
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
add_subdirectory(sys)
add_subdirectory(machine)
add_subdirectory(ssp)
add_subdirectory(rpc)
add_subdirectory(arpa)

picolibc_headers(""
  alloca.h
  _ansi.h
  argz.h
  ar.h
  assert.h
  byteswap.h
  cpio.h
  ctype.h
  devctl.h
  dirent.h
  elf.h
  endian.h
  envlock.h
  envz.h
  errno.h
  fastmath.h
  fcntl.h
  fenv.h
  fnmatch.h
  getopt.h
  glob.h
  grp.h
  iconv.h
  ieeefp.h
  inttypes.h
  langinfo.h
  libgen.h
  limits.h
  locale.h
  malloc.h
  math.h
  memory.h
  newlib.h
  paths.h
  picotls.h
  pwd.h
  regdef.h
  regex.h
  sched.h
  search.h
  setjmp.h
  signal.h
  spawn.h
  stdint.h
  stdlib.h
  string.h
  strings.h
  _syslist.h
  tar.h
  termios.h
  threads.h
  time.h
  unctrl.h
  unistd.h
  utime.h
  utmp.h
  wchar.h
  wctype.h
  wordexp.h
  )

if(${_HAVE_COMPLEX})
  picolibc_headers(""
    complex.h
    )
endif()

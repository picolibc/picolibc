#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2019 Keith Packard
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
subdir('sys')
subdir('machine')
subdir('ssp')
subdir('rpc')
subdir('arpa')

inc_headers = [
  'alloca.h',
  'argz.h',
  'ar.h',
  'assert.h',
  'byteswap.h',
  'cpio.h',
  'ctype.h',
  'devctl.h',
  'dirent.h',
  'elf.h',
  'endian.h',
  'envlock.h',
  'envz.h',
  'errno.h',
  'fastmath.h',
  'fcntl.h',
  'fenv.h',
  'fnmatch.h',
  'getopt.h',
  'glob.h',
  'grp.h',
  'iconv.h',
  'ieeefp.h',
  'inttypes.h',
  'langinfo.h',
  'libgen.h',
  'limits.h',
  'locale.h',
  'malloc.h',
  'math.h',
  'memory.h',
  'newlib.h',
  'paths.h',
  'pwd.h',
  'regdef.h',
  'regex.h',
  'sched.h',
  'search.h',
  'setjmp.h',
  'signal.h',
  'spawn.h',
  'stdint.h',
  'stdnoreturn.h',
  'stdlib.h',
  'string.h',
  'strings.h',
  '_syslist.h',
  'tar.h',
  'termios.h',
  'threads.h',
  'time.h',
  'unctrl.h',
  'unistd.h',
  'utime.h',
  'utmp.h',
  'wchar.h',
  'wctype.h',
  'wordexp.h'
]

if have_complex
  inc_headers += ['complex.h']
endif

inc_headers += ['picotls.h']

install_headers(inc_headers,
		install_dir: include_dir)

# For compatibility with libc++'s __mbstate_t.h:
install_headers(['bits/types/mbstate_t.h'],
		install_dir: include_dir / 'bits/types')

if enable_cdefs_tests

  ignore_headers = ['cpio.h', 'paths.h', 'termios.h', 'newlib.h', 'regdef.h', 'tar.h', 'utmp.h']
  foreach header : inc_headers

    if not (header in ignore_headers) and not (header.startswith('_'))
      test_name = 'check-cdef-' + header

      test(test_name,
           validate_cdefs,
           args: [meson.current_source_dir() / header] + cc.cmd_array() + c_args + inc_args,
           suite: 'headers')
    endif
  endforeach
endif

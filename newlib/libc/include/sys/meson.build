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
inc_sys_headers_all = [
  'auxv.h',
  'cdefs.h',
  'config.h',
  'custom_file.h',
  '_default_fcntl.h',
  'dirent.h',
  'dir.h',
  'errno.h',
  'fcntl.h',
  'features.h',
  'file.h',
  'iconvnls.h',
  '_initfini.h',
  '_intsup.h',
  '_locale.h',
  'lock.h',
  'param.h',
  'queue.h',
  'resource.h',
  'sched.h',
  'select.h',
  '_select.h',
  '_sigset.h',
  'stat.h',
  '_stdint.h',
  'string.h',
  'syslimits.h',
  'timeb.h',
  'time.h',
  'times.h',
  '_timespec.h',
  'timespec.h',
  '_timeval.h',
  'tree.h',
  '_types.h',
  'types.h',
  '_tz_structs.h',
  'unistd.h',
  'utime.h',
  'wait.h',
  '_wait.h'
]

inc_sys_headers = []
foreach file : inc_sys_headers_all
  if not (file in inc_sys_headers_machine)
    inc_sys_headers += file
  else
    message('libc/include/sys/' + file + ': machine overrides generic')
  endif
endforeach

install_headers(inc_sys_headers,
		install_dir: include_dir / 'sys')

if enable_cdefs_tests
  ignore_headers = ['config.h', 'features.h', 'string.h', 'custom_file.h', 'dirent.h']
  foreach header : inc_sys_headers
    if not (header in ignore_headers) and not (header.startswith('_'))
      test_name = 'check-cdef-sys-' + header
      test(test_name,
           validate_cdefs,
           args: [meson.current_source_dir() / header] + cc.cmd_array() + c_args + inc_args,
           suite: 'headers')
    endif
  endforeach
endif

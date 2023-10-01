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

libdirs = ['argz', 'ctype', 'errno', 'iconv', 'misc',
	   'posix', 'search', 'signal', 'ssp', 'stdlib',
	   'string', 'time', 'xdr', 'locale']

if enable_picolib
  libdirs += 'picolib'
endif

if tinystdio
  libdirs += 'tinystdio'
else
  libdirs += ['stdio', 'reent']
  if newlib_stdio64
    libdirs += 'stdio64'
  endif
endif

libnames = libdirs

inc_sys_headers_machine = []
inc_machine_headers_machine = []
srcs_machine = []

machine_dir = 'machine' / host_cpu_family
if fs.is_dir(machine_dir)

  machine_inc_dir = machine_dir / 'include'
  if fs.is_dir(machine_inc_dir)
    inc = [include_directories(machine_inc_dir), inc]
  endif

  inc = [include_directories(machine_dir), inc]

  libdirs = [machine_dir] + libdirs
  libnames = ['machine'] + libnames

endif

foreach libdir : libdirs
  subdir(libdir)
endforeach

src_cpart = []

foreach libname : libnames
  src_cpart += get_variable('src_' + libname, [])
endforeach

subdir('include')

foreach target : targets
  value = get_variable('target_' + target)
  libobjs = []
  libsrcs_target = []

  foreach libname : libnames
    if is_variable('lib_' + libname + target)
      libobjs += get_variable('lib_' + libname + target).extract_all_objects(recursive:true)
    endif
    libsrcs_target += get_variable('src_' + libname + target, [])
  endforeach

  set_variable('src_cpart_' + target, libsrcs_target)

  if target == ''
    libcpart_name = 'cpart'
  else
    libcpart_name = 'cpart' + '_' + target
  endif

  local_lib_cpart_target = static_library(libcpart_name,
				          pic: false,
				          objects : libobjs,
					  include_directories: inc,
					  c_args: value[1] + c_args)

  set_variable('lib_cpart' + target, local_lib_cpart_target)
endforeach

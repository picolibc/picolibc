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
subdir('libc')
subdir('libm')

libnames = ['cpart', 'mpart']

libsrcs = []

foreach libname : libnames
  libsrcs += get_variable('src_' + libname, [])
endforeach

foreach target : targets
  value = get_variable('target_' + target)
  libobjs = []
  libsrcs_target = libsrcs

  foreach libname : libnames
    if is_variable('lib_' + libname + target)
      libobjs += get_variable('lib_' + libname + target).extract_all_objects(recursive:true)
    endif
    libsrcs_target += get_variable('src_' + libname + target, [])
  endforeach

  instdir = join_paths(lib_dir, value[0])

  if target == ''
    libc_name = 'c'
    libg_name = 'g'
    libm_name = 'm'
  else
    libc_name = join_paths(target, 'libc')
    libg_name = join_paths(target, 'libg')
    libm_name = join_paths(target, 'libm')
  endif

  local_lib_c_target = static_library(libc_name,
				      libsrcs_target,
				      install : true,
				      install_dir : instdir,
				      pic: false,
				      objects : libobjs,
				      include_directories: inc,
				      c_args: value[1] + c_args)

  set_variable('lib_c' + target, local_lib_c_target)
  static_library(libm_name,
		 ['empty.c'],
		 install : true,
		 install_dir : instdir,
		 pic: false,
                 c_args: value[1] + c_args,
		 objects : [])
  static_library(libg_name,
		 ['empty.c'],
		 install : true,
		 install_dir : instdir,
		 pic: false,
                 c_args: value[1] + c_args,
		 objects : [])

  if check_duplicate_names
    custom_target('libc_duplicates' + target,
		  build_by_default: true,
		  input: local_lib_c_target,
		  output: 'libc_duplicates' + target,
		  command: [duplicate_names, nm, '@INPUT@', '@OUTPUT@'],
		  install: false)
  endif
endforeach

if enable_tests
  subdir('testsuite')
  subdir('libm/test')
endif

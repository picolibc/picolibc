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
libc_tests_common = [
  'basename',
  'dirname',
  'fnmatch',
  'qsort',
  'snprintf',
  'sscanf',
  'string',
  'strtol',
  'wcstol',
]

libc_tests_picolibc = libc_tests_common

if io_float_exact
  libc_tests_picolibc += ['strtod']
endif

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  _libs = [get_variable('lib_c' + target)]
  if is_variable(oslib_test + target)
    _libs += [get_variable(oslib_test + target)]
  endif

  if is_variable(crt0_test + target)
    _objs = [get_variable(crt0_test + target)]
  else
    _objs = []
  endif

  _lib_files=[]
  foreach _lib : _libs
    _lib_files += _lib.full_path()
  endforeach

  _c_args = target_c_args + get_variable('test_c_args' + target, test_c_args)
  _link_args = target_c_args + _lib_files + get_variable('test_link_args' + target, test_link_args)
  _link_depends = get_variable('test_link_depends' + target, test_link_depends)

  foreach t1 : libc_tests_picolibc
    t1_src = t1 + '.c'

    timeout = 30
    if t1 == 'snprintf'
      # snprintf has one test which is really slow
      timeout = 900
    endif

    test(t1 + target,
	 executable(t1 + target, [t1_src],
		    c_args: printf_compile_args_d + _c_args,
		    link_args: printf_link_args_d + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
                    link_with: _libs,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: timeout,
         suite: 'libc-testsuite',
	 env: test_env)
  endforeach
endforeach

if enable_native_tests

  libc_tests_native = libc_tests_common
  libc_tests_native_dependencies = []

  if enable_native_math_tests
    libc_tests_native += ['strtod']
    libc_tests_native_dependencies += native_lib_m
  endif

  foreach t1 : libc_tests_native
    t1_src = t1 + '.c'

    test(t1 + '-native',
	 executable(t1 + '-native', t1_src,
		    c_args: native_c_args,
		    link_args: native_c_args,
                    dependencies: libc_tests_native_dependencies))
  endforeach

endif


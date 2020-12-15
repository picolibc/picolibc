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
foreach target : targets
  value = get_variable('target_' + target)

  _libs = [get_variable('lib_m' + target), get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    _libs += [get_variable('lib_semihost' + target)]
  endif
  if is_variable('lib_crt' + target)
    _libs += [get_variable('lib_crt'+ target)]
  endif
  
  _c_args = value[1] + test_c_args
  _link_args = value[1] + test_link_args

  t1 = 'printf_scanf'
  if target == ''
    t1_name = t1
  else
    t1_name = join_paths(target, t1)
  endif

  test(t1 + target,
       executable(t1_name, 'printf_scanf.c',
		  c_args: _c_args,
		  link_args: _link_args,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       env: ['MESON_SOURCE_ROOT=' + meson.source_root()])

  t1 = 'printff_scanff'
  if target == ''
    t1_name = t1
  else
    t1_name = join_paths(target, t1)
  endif

  test(t1 + target,
       executable(t1_name, 'printf_scanf.c',
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       env: ['MESON_SOURCE_ROOT=' + meson.source_root()])

  t1 = 'printf-tests'
  if target == ''
    t1_name = t1
  else
    t1_name = join_paths(target, t1)
  endif

  test(t1 + target,
       executable(t1_name, ['printf-tests.c'],
		  c_args: _c_args,
		  link_args: _link_args,
		  link_with: _libs,
		  include_directories: inc),
       env: ['MESON_SOURCE_ROOT=' + meson.source_root()])

  t1 = 'printff-tests'
  if target == ''
    t1_name = t1
  else
    t1_name = join_paths(target, t1)
  endif

  test(t1 + target,
       executable(t1_name, ['printf-tests.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  link_with: _libs,
		  include_directories: inc),
       env: ['MESON_SOURCE_ROOT=' + meson.source_root()])

  t1 = 'try-ilp32'
  if target == ''
    t1_name = t1
  else
    t1_name = join_paths(target, t1)
  endif

  test(t1 + target,
       executable(t1_name, ['try-ilp32.c', 'try-ilp32-sub.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  link_with: _libs,
		  include_directories: inc),
       env: ['MESON_SOURCE_ROOT=' + meson.source_root()])

  plain_tests = ['rand', 'regex', 'ungetc', 'fenv', 'math_errhandling', 'malloc', 'tls', 'ffs', 'setjmp']

  if newlib_nano_malloc or not tests_disable_full_malloc_stress
    plain_tests += 'malloc_stress'
  endif

  if posix_io or not tinystdio
    plain_tests += 'posix-io'
  endif

  if tests_enable_stack_protector
    plain_tests += 'stack-smash'
  endif

  foreach t1 : plain_tests
    t1_src = t1 + '.c'
    if target == ''
      t1_name = t1
    else
      t1_name = join_paths(target, t1)
    endif

    test(t1 + target,
	 executable(t1_name, [t1_src],
		    c_args: _c_args,
		    link_args: _link_args,
		    link_with: _libs,
		    link_depends:  test_link_depends,
		    include_directories: inc),
	 env: ['MESON_SOURCE_ROOT=' + meson.source_root()])
  endforeach

endforeach

if enable_native_tests

  native_lib_m = meson.get_compiler('c').find_library('m', required: false)

  if native_lib_m.found()
    test('math_errhandling_native',
	 executable('math_errhandling_native', 'math_errhandling.c',
		    c_args: test_c_warnings + ['-D_XOPEN_SOURCE=700', '-D_GNU_SOURCE', '-DNO_NEWLIB'],
		    dependencies: native_lib_m))
  endif
endif

if has_semihost
  subdir('semihost')
endif

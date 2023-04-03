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

  _libs = [get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    _libs += [get_variable('lib_semihost' + target)]
  endif

  if is_variable('crt0_semihost' + target)
    _objs = [get_variable('crt0_semihost'+ target)]
  elif is_variable('crt0_hosted' + target)
    _objs = [get_variable('crt0_hosted'+ target)]
  else
    _objs = []
  endif

  have_crt_minimal = is_variable('crt0_minimal' + target)
  if have_crt_minimal
    _objs_minimal = [get_variable('crt0_minimal'+ target)]
  else
    _objs_minimal = _objs
  endif

  _c_args = value[1] + test_c_args
  _link_args = value[1] + test_link_args

  t1 = 'printf_scanf'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printff_scanff'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printf-tests'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['printf-tests.c', 'lock-valid.c'],
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printff-tests'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['printf-tests.c', 'lock-valid.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfi-tests'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['printf-tests.c', 'lock-valid.c'],
		  c_args: int_printf_compile_args + _c_args,
		  link_args: int_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'try-ilp32'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  t1 = 'time-sprintf'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['time-sprintf.c', 'lock-valid.c'],
		  c_args: int_printf_compile_args + _c_args + ['-fno-builtin'],
		  link_args: int_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'try-ilp32'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['try-ilp32.c', 'try-ilp32-sub.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'hosted-exit'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args:  _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  test(t1 + '-fail' + target,
       executable(t1_name + '-fail', ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args + ['-DRETVAL=1'],
		  link_args:  _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  t1 = 'abort'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['abort.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args:  _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  if have_crt_minimal
    t1 = 'constructor-skip'
    if target == ''
      t1_name = t1
    else
      t1_name = t1 + '_' + target
    endif

    test(t1_name,
	 executable(t1_name, ['constructor-skip.c', 'lock-valid.c'],
		    c_args: _c_args,
		    link_args:  _link_args,
		    objects: _objs_minimal,
		    link_with: _libs,
		    include_directories: inc),
         depends: bios_bin,
	 env: test_env)
  endif

  t1 = 'math_errhandling'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['math_errhandling.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'rounding-mode'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, ['rounding-mode.c', 'rounding-mode-sub.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'test-except'
  t1_src = t1 + '.c'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, [t1_src, 'lock-valid.c'],
		  c_args: double_printf_compile_args + arg_fnobuiltin + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  t1 = 'long_double'
  t1_src = t1 + '.c'
  if target == ''
    t1_name = t1
  else
    t1_name = t1 + '_' + target
  endif

  test(t1_name,
       executable(t1_name, t1_src,
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_with: _libs,
		  link_depends:  test_link_depends,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
      )

  plain_tests = ['rand', 'regex', 'ungetc', 'fenv',
		 'malloc', 'tls',
		 'ffs', 'setjmp', 'atexit', 'on_exit',
		 'math-funcs', 'timegm', 'time-tests',
                 'test-strtod', 'test-strchr',
		 'test-memset', 'test-put',
		 'test-efcvt', 'test-atomic',
		 'test-raise',
		]

  if have_attr_ctor_dtor
    plain_tests += 'constructor'
  endif

  if have_complex
    plain_tests += 'complex-funcs'
  endif

  if newlib_nano_malloc or tests_enable_full_malloc_stress
    plain_tests += 'malloc_stress'
  endif

  if (posix_io or not tinystdio) and tests_enable_posix_io
    plain_tests += ['posix-io']

    # legacy stdio doesn't work on semihosting, so just skip it
    if tinystdio
      plain_tests += ['test-fopen', 'test-mktemp']
    endif
  endif

  if tests_enable_stack_protector
    plain_tests += 'stack-smash'
  endif

  foreach t1 : plain_tests
    t1_src = t1 + '.c'
    if target == ''
      t1_name = t1
    else
      t1_name = t1 + '_' + target
    endif

    test(t1_name,
	 executable(t1_name, [t1_src, 'lock-valid.c'],
		    c_args: double_printf_compile_args + _c_args,
		    link_args: double_printf_link_args + _link_args,
		    objects: _objs,
		    link_with: _libs,
		    link_depends:  test_link_depends,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 60,
	 env: test_env)
  endforeach

endforeach

if enable_native_tests

  native_lib_m = cc.find_library('m', required: false)

  if native_lib_m.found()
    test('long_double_native',
	 executable('long_double_native', 'long_double.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('math_errhandling_native',
	 executable('math_errhandling_native', 'math_errhandling.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('rounding-mode_native',
	 executable('rounding-mode_native',
		    ['rounding-mode.c', 'rounding-mode-sub.c'],
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('printf-tests_native',
	 executable('printf-tests_native',
		    'printf-tests.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('printf_scanf_native',
	 executable('printf_scanf_native',
		    'printf_scanf.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('test-efcvt_native',
	 executable('test-efcvt_native',
		    'test-efcvt.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
    test('test-strtod_native',
	 executable('test-strtod_native',
		    'test-strtod.c',
		    c_args: native_c_args,
		    dependencies: native_lib_m))
  endif

  test('test-atomic-native',
       executable('test-atomic-native', 'test-atomic.c',
		  c_args: native_c_args))

endif

subdir('libc-testsuite')

if has_arm_semihost
  subdir('semihost')
endif

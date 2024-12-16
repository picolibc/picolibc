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

plain_tests_common = ['regex', 'ungetc',
	              'malloc',
	              'ffs', 'setjmp', 'atexit', 'on_exit',
	              'timegm',
                      'test-strchr',
	              'test-memset', 'test-put',
	              'test-raise',
                      'test-sprintf-percent-n',
                      'test-ctype',
                      'test-uchar',
	      ]

math_tests_common = [
  'fenv',
  'rand',
  'test-efcvt',
  'test-fma',
  'test-strtod',
]

if have_attr_ctor_dtor
  plain_tests_common += 'constructor'
endif

if have_complex
  math_tests_common += 'complex-funcs'
endif

if newlib_nano_malloc or tests_enable_full_malloc_stress
  math_tests_common += 'malloc_stress'
endif

if tinystdio
  plain_tests_common += [
    't_fmemopen',
    'test-vfscanf-percent-a',
  ]
endif

if atomic_ungetc or atomic_signal
  plain_tests_common += 'test-atomic'
endif

if (posix_io or not tinystdio) and tests_enable_posix_io
  plain_tests_common += ['posix-io']

  # legacy stdio doesn't work on semihosting, so just skip it
  if tinystdio
    plain_tests_common += ['test-fopen',
                    'test-mktemp',
                    'test-tmpnam',
                    'test-fread-fwrite',
                    'test-ungetc-ftell',
                    'test-fgetc',
                    'test-fgets-eof',
                    'test-wchar',
                   ]
  endif
endif

math_tests = math_tests_common + [
  'math-funcs',
]

plain_tests = plain_tests_common

plain_tests += math_tests + [
  'test-funopen',
  'time-tests',
  'tls',  
]

plain_tests += ['test-memcpy_s',
                'test-memset_s',
                'test-memmove_s',
                'test-strcat_s',
                'test-strcpy_s',
                'test-strerror_s',
                'test-strerrorlen_s',
                'test-strncat_s',
                'test-strncpy_s',
                'test-strnlen_s',
               ]

if tinystdio
  plain_tests += 'test-sprintf_s'
endif

if tests_enable_stack_protector
  plain_tests += 'stack-smash'
endif

plain_tests_native = plain_tests_common

math_tests_native = math_tests_common
foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']
  target_lib_prefix = params['lib_prefix']

  _libs = [get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    _libs += [get_variable('lib_semihost' + target)]
  endif

  _lib_files=[]
  foreach _lib : _libs
    _lib_files += _lib.full_path()
  endforeach

  if is_variable(crt0_test + target)
    _objs = [get_variable(crt0_test + target)]
  else
    _objs = []
  endif

  _c_args = target_c_args + get_variable('test_c_args' + target, test_c_args)
  _link_args = target_c_args + _lib_files + get_variable('test_link_args' + target, test_link_args)
  _link_depends = get_variable('test_link_depends' + target, test_link_depends)
  if have_cplusplus
    _cpp_args = target_c_args + get_variable('test_cpp_args_' + target, test_cpp_args)
  endif

  t1 = 'printf_scanf'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printff_scanff'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfl_scanfl'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: llong_printf_compile_args + _c_args,
		  link_args: llong_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfi_scanfi'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: int_printf_compile_args + _c_args,
		  link_args: int_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfm_scanfm'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: min_printf_compile_args + _c_args,
		  link_args: min_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printf-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printff-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: float_printf_compile_args + _c_args,
		  link_args: float_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfi-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: int_printf_compile_args + _c_args,
		  link_args: int_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'printfm-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: min_printf_compile_args + _c_args,
		  link_args: min_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'time-sprintf'

  test(t1 + target,
       executable(t1 + target, ['time-sprintf.c', 'lock-valid.c'],
		  c_args: int_printf_compile_args + _c_args + ['-fno-builtin'],
		  link_args: int_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'try-ilp32'

  test(t1 + target,
       executable(t1 + target, ['try-ilp32.c', 'try-ilp32-sub.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'hosted-exit'
  t1_fail = t1 + '-fail'

  test(t1 + target,
       executable(t1 + target, ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args:  _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  test(t1_fail + target,
       executable(t1_fail + target, ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args + ['-DRETVAL=1'],
		  link_args:  _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  t1 = 'abort'

  test(t1 + target,
       executable(t1 + target, ['abort.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args:  _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  # POSIX console code requires a constructor to run, so
  # it is incompatible with the minimal crt0 when using stdout
  # (which lock-valid.c uses).

  if is_variable('crt0_minimal' + target) and not posix_console
    _objs_minimal = [get_variable('crt0_minimal'+ target)]
    t1 = 'constructor-skip'

    test(t1 + target,
	 executable(t1 + target, ['constructor-skip.c', 'lock-valid.c'],
		    c_args: _c_args,
		    link_args:  _link_args,
		    objects: _objs_minimal,
		    link_depends:  _link_depends,
		    link_with: _libs,
		    include_directories: inc),
         depends: bios_bin,
	 env: test_env)
  endif

  t1 = 'math_errhandling'

  test(t1 + target,
       executable(t1 + target, ['math_errhandling.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'rounding-mode'

  test(t1 + target,
       executable(t1 + target, ['rounding-mode.c', 'rounding-mode-sub.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  t1 = 'test-except'
  t1_src = t1 + '.c'

  test(t1 + target,
       executable(t1 + target, [t1_src, 'lock-valid.c'],
		  c_args: double_printf_compile_args + arg_fnobuiltin + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       should_fail: true
      )

  t1 = 'long_double'
  t1_src = t1 + '.c'

  test(t1 + target,
       executable(t1 + target, t1_src,
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       env: test_env,
       timeout: 90,
      )

  if get_option('test-stdin')
    t1 = 'test-gets'
    t1_src = 'test-gets.c'

    test_gets = executable(t1 + target, t1_src,
		           c_args: _c_args,
		           link_args: _link_args,
		           objects: _objs,
		           link_depends:  _link_depends,
		           link_with: _libs,
		           include_directories: inc)

    test(t1 + target + '-success',
         test_gets,
         depends: bios_bin,
         args: ['hi'],
         env: test_env,
         timeout: 90
        )


    test(t1 + target + '-failure',
         test_gets,
         depends: bios_bin,
         args: ['this is a long string that should overflow the buffer'],
         env: test_env,
         timeout: 90,
         should_fail: true
        )
  endif

  if have_ubsan
    t1 = 'test-ubsan'
    t1_src = t1 + '.c'

    test_ubsan_flags = []
    if sanitize_trap_on_error
      test_ubsan_flags = ['-DSANITIZE_TRAP_ON_ERROR']
    endif

    test(t1 + target,
	 executable(t1 + target, t1_src,
		    c_args: c_sanitize_bounds_flags + _c_args + test_ubsan_flags,
		    link_args: _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    link_with: _libs,
		    include_directories: inc),
	 depends: bios_bin,
	 env: test_env,
	 should_fail: sanitize_trap_on_error
	)
  endif

  t1 = 'test-long-longl'

  test(t1 + target,
       executable(t1 + target, ['test-long-long.c', 'lock-valid.c'],
		  c_args: llong_printf_compile_args + _c_args,
		  link_args: llong_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       timeout: 120,
       priority: -1,
       env: test_env)

  t1 = 'test-long-long'

  test(t1 + target,
       executable(t1 + target, ['test-long-long.c', 'lock-valid.c'],
		  c_args: double_printf_compile_args + _c_args,
		  link_args: double_printf_link_args + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  link_with: _libs,
		  include_directories: inc),
       depends: bios_bin,
       timeout: 120,
       priority: -1,
       env: test_env)

  foreach t1 : plain_tests
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']

    test(t1 + target,
	 executable(t1 + target, [t1_src, 'lock-valid.c'],
		    c_args: double_printf_compile_args + test_file_name_arg + _c_args,
		    link_args: double_printf_link_args + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    link_with: _libs,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 60,
	 env: test_env)
  endforeach

  if have_cplusplus
    t1 = 'test-cplusplus'
    t1_src = t1 + '.cpp'
    test(t1 + target,
         executable(t1 + target, t1_src,
                    cpp_args: _cpp_args,
		    link_args: _cpp_args + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    link_with: _libs,
                    include_directories: inc))
  endif
endforeach

if enable_native_math_tests

  native_lib_m = cc.find_library('m', required: false)

  if native_lib_m.found()
    test('long_double-native',
	 executable('long_double-native', 'long_double.c',
		    c_args: native_c_args,
		    link_args: native_c_args,
		    dependencies: native_lib_m))
    test('math_errhandling-native',
	 executable('math_errhandling-native', 'math_errhandling.c',
		    c_args: native_c_args,
		    link_args: native_c_args,
		    dependencies: native_lib_m))
    test('rounding-mode-native',
	 executable('rounding-mode-native',
		    ['rounding-mode.c', 'rounding-mode-sub.c'],
		    c_args: native_c_args,
		    link_args: native_c_args,
		    dependencies: native_lib_m))
    test('printf-tests-native',
	 executable('printf-tests-native',
		    'printf-tests.c',
		    c_args: native_c_args,
		    link_args: native_c_args,
		    dependencies: native_lib_m))
    test('printf_scanf-native',
	 executable('printf_scanf-native',
		    'printf_scanf.c',
		    c_args: native_c_args,
		    link_args: native_c_args,
		    dependencies: native_lib_m))

    foreach t1 : math_tests_native
      t1_src = t1 + '.c'

      test(t1 + '-native',
	   executable(t1 + '-native', t1_src,
		      c_args: native_c_args,
		      link_args: native_c_args,
                      dependencies: native_lib_m))
    endforeach

  endif


endif

if enable_native_tests

  if have_cplusplus
    test('test-cplusplus-native',
         executable('test-cplusplus-native', 'test-cplusplus.cpp',
                    cpp_args: native_cpp_args,
		    link_args: native_cpp_args))
  endif

  
  foreach t1 : plain_tests_native
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']

    test(t1 + target,
	 executable(t1 + '-native', t1_src,
		    c_args: native_c_args,
		    link_args: native_c_args))
  endforeach

  if newlib_mb
    diff = find_program('diff', required: false)

    if diff.found()
      wctype = executable('test-wctype', 'test-wctype.c',
                          c_args: _c_args,
                          link_args: _link_args,
                          objects: _objs,
                          link_depends: _link_depends,
                          link_with: _libs,
                          include_directories: inc)

      wctype_native = executable('test-wctype-native', 'test-wctype.c',
                                 c_args: native_c_args,
                                 link_args: native_c_args)

      wctype_out = custom_target ('wctype-out', output: 'wctype-out',
                                  command: [wctype, '@OUTPUT@'])

      wctype_native_out = custom_target ('wctype-native-out', output: 'wctype-native-out',
                                         command: [wctype_native, '@OUTPUT@'])

      test('wctype-compare', diff, args: [wctype_out, wctype_native_out])
    endif
  endif
       
endif

subdir('libc-testsuite')

if has_arm_semihost
  subdir('semihost')
endif

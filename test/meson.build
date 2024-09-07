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
                      'test-atomic',
                      'test-strchr',
	              'test-memset', 'test-put',
	              'test-raise',
                      'test-sprintf-percent-n',
                      'test-ctype',
                      'test-uchar',
                      'test-wctomb',
                      'test-scmpu',
                      'test-strncpy',
                      'time-tests',
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

if tests_enable_posix_io
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

plain_tests_native += 'test-flockfile'

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
  _link_depends = get_variable('test_link_depends' + target, test_link_depends) + _libs
  if have_cplusplus
    _cpp_args = target_c_args + get_variable('test_cpp_args_' + target, test_cpp_args)
  endif

  t1 = 'printf_scanf'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_d + _c_args,
		  link_args: printf_link_args_d + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printff_scanff'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_f + _c_args,
		  link_args: printf_link_args_f + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printfl_scanfl'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_l + _c_args,
		  link_args: printf_link_args_l + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printfi_scanfi'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_i + _c_args,
		  link_args: printf_link_args_i + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printfm_scanfm'

  test(t1 + target,
       executable(t1 + target, ['printf_scanf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_m + _c_args,
		  link_args: printf_link_args_m + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printf-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: printf_compile_args_d + _c_args,
		  link_args: printf_link_args_d + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printff-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: printf_compile_args_f + _c_args,
		  link_args: printf_link_args_f + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printfi-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: printf_compile_args_i + _c_args,
		  link_args: printf_link_args_i + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'printfm-tests'

  test(t1 + target,
       executable(t1 + target, ['printf-tests.c', 'lock-valid.c'],
		  c_args: printf_compile_args_m + _c_args,
		  link_args: printf_link_args_m + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'time-sprintf'

  test(t1 + target,
       executable(t1 + target, ['time-sprintf.c', 'lock-valid.c'],
		  c_args: printf_compile_args_i + _c_args + ['-fno-builtin'],
		  link_args: printf_link_args_i + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'try-ilp32'

  test(t1 + target,
       executable(t1 + target, ['try-ilp32.c', 'try-ilp32-sub.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'hosted-exit'
  t1_fail = t1 + '-fail'

  test(t1 + target,
       executable(t1 + target, ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args,
		  link_args:  _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  test(t1_fail + target,
       executable(t1_fail + target, ['hosted-exit.c', 'lock-valid.c'],
		  c_args: _c_args + ['-DRETVAL=1'],
		  link_args:  _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
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
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
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
		    include_directories: inc),
         depends: bios_bin,
         suite: 'test',
	 env: test_env)
  endif

  t1 = 'math_errhandling'

  test(t1 + target,
       executable(t1 + target, ['math_errhandling.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'rounding-mode'

  test(t1 + target,
       executable(t1 + target, ['rounding-mode.c', 'rounding-mode-sub.c'],
		  c_args: arg_fnobuiltin + _c_args,
		  link_args: _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env)

  t1 = 'test-except'
  t1_src = t1 + '.c'

  test(t1 + target,
       executable(t1 + target, [t1_src, 'lock-valid.c'],
		  c_args: printf_compile_args_d + arg_fnobuiltin + _c_args,
		  link_args: printf_link_args_d + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
       env: test_env,
       should_fail: true
      )

  t1 = 'long_double'
  t1_src = t1 + '.c'

  test(t1 + target,
       executable(t1 + target, t1_src,
		  c_args: printf_compile_args_d + _c_args,
		  link_args: printf_link_args_d + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       suite: 'test',
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
		           include_directories: inc)

    test(t1 + target + '-success',
         test_gets,
         depends: bios_bin,
         args: ['hi'],
         suite: 'test',
         env: test_env,
         timeout: 90
        )


    test(t1 + target + '-failure',
         test_gets,
         depends: bios_bin,
         args: ['this is a long string that should overflow the buffer'],
         suite: 'test',
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
		    include_directories: inc),
	 depends: bios_bin,
         suite: 'test',
	 env: test_env,
	 should_fail: sanitize_trap_on_error
	)
  endif

  t1 = 'test-long-longl'

  test(t1 + target,
       executable(t1 + target, ['test-long-long.c', 'lock-valid.c'],
		  c_args: printf_compile_args_l + _c_args,
		  link_args: printf_link_args_l + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       timeout: 120,
       priority: -1,
       suite: 'test',
       env: test_env)

  t1 = 'test-long-long'

  test(t1 + target,
       executable(t1 + target, ['test-long-long.c', 'lock-valid.c'],
		  c_args: printf_compile_args_d + _c_args,
		  link_args: printf_link_args_d + _link_args,
		  objects: _objs,
		  link_depends:  _link_depends,
		  include_directories: inc),
       depends: bios_bin,
       timeout: 120,
       priority: -1,
       suite: 'test',
       env: test_env)

  foreach t1 : plain_tests
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']

    test(t1 + target,
	 executable(t1 + target, [t1_src, 'lock-valid.c'],
		    c_args: printf_compile_args_d + test_file_name_arg + _c_args,
		    link_args: printf_link_args_d + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 60,
         suite: 'test',
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
                    include_directories: inc),
         depends: bios_bin,
         suite: 'test',
         env: test_env)
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

  localedef = find_program('localedef', required: false)

  ctype_c_args = []

  locales=[]
  # need meson 0.57 so we can use 'env' in custom_target
  if localedef.found() and meson.version().version_compare('>=0.57')
    if mb_capable
      locale = 'fr_FR.UTF-8'
      locales += [custom_target(locale, output: locale,
                                command: [localedef, '--no-warnings=ascii',
                                          '-f', 'UTF-8', '-i', 'fr_FR', '.' / '@OUTPUT@'])]
      locale = 'C.UTF-8'
      locales += [custom_target(locale, output: locale,
                                command: [localedef, '--no-warnings=ascii',
                                          '-f', 'UTF-8', '-i', 'C', '.' / '@OUTPUT@'])]
      ctype_c_args += ['-DHAVE_UTF_CHARSETS']
    endif
    if mb_extended_charsets
      # CP720 doesn't have any Linux support.
      # JIS doesn't make any sense to me.
      # CP1255 and CP1258 seem to be broken in Linux
      foreach ctype : ['ISO-8859-1', 'ISO-8859-2', 'ISO-8859-3', 'ISO-8859-4',
                       'ISO-8859-5', 'ISO-8859-6', 'ISO-8859-7', 'ISO-8859-8',
                       'ISO-8859-9', 'ISO-8859-10', 'ISO-8859-11', 'ISO-8859-13',
                       'ISO-8859-14', 'ISO-8859-15', 'ISO-8859-16',

                       'GEORGIAN-PS', 'PT154', 'KOI8-T', 'CP437', 'CP737', 'CP775',
                       'CP850', 'CP852', 'CP855', 'CP857', 'CP858', 'CP862', 'CP866',
                       'CP874', 'CP1125', 'CP1250', 'CP1251', 'CP1252', 'CP1253',
                       'CP1254', 'CP1256', 'CP1257', 'KOI8-R', 'KOI8-U',

                       'SHIFT-JIS', 'EUC-JP']
        locale = 'C.' + ctype
        if ctype == 'SHIFT-JIS'
          ctype = 'SHIFT_JIS'
        elif ctype == 'CP437'
          ctype = 'IBM437'
        elif ctype == 'CP850'
          ctype = 'IBM850'
        elif ctype == 'CP852'
          ctype = 'IBM852'
        elif ctype == 'CP855'
          ctype = 'IBM855'
        elif ctype == 'CP857'
          ctype = 'IBM857'
        elif ctype == 'CP858'
          ctype = 'IBM858'
        elif ctype == 'CP862'
          ctype = 'IBM862'
        elif ctype == 'CP866'
          ctype = 'IBM866'
        elif ctype == 'CP874'
          ctype = 'IBM874'
        endif
        locales += [custom_target(locale, output: locale,
                                  command: [localedef, '--no-warnings=ascii',
                                            '-f', ctype, '-i', 'C', '.' / '@OUTPUT@'])]
      endforeach
      ctype_c_args += ['-DHAVE_ISO_CHARSETS',
                       '-DHAVE_WINDOWS_CHARSETS',
                       '-DHAVE_JIS_CHARSETS'
                      ]
    endif
  endif

  native_lib = static_library('native-lib',
                              ['native-locks.c'],
                              c_args: native_c_args)

  test_file_name_arg=['-DTEST_FILE_NAME="' + 'test-flockfile' + '.txt"']

  test('test-flockfile',
       executable('test-flockfile',
                  'test-flockfile.c',
                  c_args: test_c_args + test_file_name_arg,
                  link_args: test_link_args,
                  link_with: [native_lib, lib_c],
                  include_directories: inc),
       depends: bios_bin,
       env: test_env)

  if have_cplusplus
    test('test-cplusplus-native',
         executable('test-cplusplus-native', 'test-cplusplus.cpp',
                    cpp_args: native_cpp_args,
		    link_args: native_cpp_args))
  endif

  foreach t1 : plain_tests_native
    t1_src = t1 + '.c'
    t1_name = t1 + '-native'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1_name + '.txt"']

    test(t1_name,
	 executable(t1_name, t1_src,
		    c_args: native_c_args + ctype_c_args,
		    link_args: native_c_args),
         env: ['LOCPATH=' + meson.current_build_dir()])
  endforeach

  diff = find_program('diff', required: false)

  if diff.found()
    diff_tests = ['test-wctype', 'test-encode', 'test-wcase']

    foreach t : diff_tests
      src = t + '.c'
      n = t + '-native'
      o = t + '-out'
      no = n + '-out'

      test = t + '-compare'

      e = executable(t, src, 
                 c_args: printf_compile_args_d + _c_args + ctype_c_args,
                 link_args: printf_link_args_d + _link_args,
                 objects: _objs,
                 link_depends: _link_depends,
                 include_directories: inc)

      out = custom_target (o, output: o,
                           command: [e, '@OUTPUT@'])

      ne = executable(n, src, 
                      c_args: native_c_args + ctype_c_args,
                      link_args: native_c_args)

      if locales != [] and meson.version().version_compare('>=0.57')
        native_out = custom_target (no, output: no,
                                    depends: locales,
                                    env: ['LOCPATH=' + meson.current_build_dir()],
                                    command: [ne, '@OUTPUT@'])
      else
        native_out = custom_target (no, output: no,
                                    command: [ne, '@OUTPUT@'])
      endif

      test(test, diff, args: ['-u', out, native_out])
    endforeach
  endif
endif

subdir('libc-testsuite')

if has_arm_semihost
  subdir('semihost')
endif

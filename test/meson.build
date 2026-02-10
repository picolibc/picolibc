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

tests = [
  'test-argv',
  'test-atomic',
  'test-environ',
  'test-except',
  'test-getdate',
  'test-getopt',
  'test-hello',
  'test-hosted-exit',
  'test-hosted-exit-fail',
  'test-ilp32',
  'test-raise',
  'test-regex',
  'test-scmpu',
  'test-setjmp',
  'test-strftime',
  'test-time',
  'test-time2',
  'test-timegm',
  'test-tls',
  'test-uchar',
  'test-wcsftime',
  'test-wctomb',
]

tests_should_fail = [
  'test-except',
  'test-hosted-exit-fail',
  ]

if have_attr_ctor_dtor
  tests += ['constructor']
endif

if tests_enable_stack_protector
  tests += ['test-stack-smash']
endif

test_ubsan_cflags = []
if c_sanitize_flags != []
  tests += ['test-ubsan']
  if sanitize_trap_on_error
    test_ubsan_cflags = ['-DSANITIZE_TRAP_ON_ERROR']
    tests_should_fail += ['test-ubsan']
  endif
endif

no_flash_tests = []
if test_noflash
  no_flash_tests = [
    'test-ilp32',
    'test-hosted-exit',
    'test-except',
    'test-hello-raw',
    'test-ubsan',
    'test-tls',
    'test-argv',
    'test-cplusplus',
  ]
endif

test_extra_libs = {
}

test_extra_src = {
  'test-ilp32': ['test-ilp32-sub.c']
}

test_extra_c_args = {
  'test-argv': ['-DEXPECT_PROG="test-argv"'],
  'test-argv-no-flash': ['-DEXPECT_PROG="test-argv-no-flash"'],
  'test-ubsan': test_ubsan_cflags,
  'test-ubsan-no-flash': test_ubsan_cflags,
}

test_args = {
  'test-argv': ['--args', 'hello world'],
  'test-argv-no-flash': ['--args', 'hello world'],
}

test_skip_native = [
  'test-stack-smash',
  'test-tls',
  'test-ubsan',
]

test_skip_lockvalid = [
  'test-hello'
  ]

foreach params : targets
  target = params['name']
  target_dir = params['dir']
  target_c_args = params['c_args']

  _libs = [get_variable('lib_c' + target)]
  if is_variable(oslib_test + target)
    _libs += [get_variable(oslib_test + target)]
  endif
  if is_variable(libfallback_var + target)
    _libs += [get_variable(libfallback_var + target)]
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
    _cpp_args = target_c_args + get_variable('test_cpp_args' + target, test_cpp_args)
  endif

  if test_noflash
    _test_link_args_noflash_variable = 'test_link_args_noflash' + target
    _link_args_no_flash = target_c_args + _lib_files + get_variable('test_link_args_noflash' + target, test_link_args_noflash)
    _link_depends_no_flash = get_variable('test_link_depends_noflash' + target, test_link_depends_noflash) + _libs
    _objs_no_flash = get_variable('crt0_noflash_semihost' + target, get_variable('crt0_noflash_semihost' + target))
  endif

  # POSIX console code requires a constructor to run, so
  # it is incompatible with the minimal crt0 when using stdout
  # (which lock-valid.c uses).

  if is_variable('crt0_minimal' + target) and not posix_console and crt0_test_variant == 'semihost'
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
         should_fail: t1 in tests_should_fail,
         args: test_args.get(t1, []),
	 env: test_env)
  endif

  if is_variable('crt0_inittls_semihost' + target) and oslib_test_variant == 'semihost'
    _link_inittls_args = target_c_args + _lib_files + get_variable('test_inittls_link_args' + target, test_link_args)
    _link_inittls_depends = get_variable('test_inittls_link_depends' + target, test_inittls_link_depends) + _libs
    _objs_inittls = [get_variable('crt0_inittls_semihost'+ target)]
    t1 = 'test-tls-init'

    test(t1 + target,
	 executable(t1 + target, ['test-tls.c', 'lock-valid.c'],
		    c_args: _c_args + ['-DINIT_TLS'],
		    link_args:  _link_inittls_args,
		    objects: _objs_inittls,
		    link_depends:  _link_inittls_depends,
		    include_directories: inc),
         depends: bios_bin,
         suite: 'test',
         should_fail: t1 in tests_should_fail,
         args: test_args.get(t1, []),
	 env: test_env)
  endif

  if is_variable('lib_semihost_raw' + target) and test_machine == 'qemu'

    _libs_raw = [get_variable('lib_c' + target)]
    _libs_raw += [get_variable('lib_semihost_raw' + target)]
    
    _lib_raw_files=[]
    foreach _lib_raw : _libs_raw
      _lib_raw_files += _lib_raw.full_path()
    endforeach

    _link_raw_args = target_c_args + _lib_raw_files + get_variable('test_link_args' + target, test_link_args)
    _link_raw_depends = get_variable('test_link_depends' + target, test_link_depends) + _libs_raw

    if is_variable('crt0_hosted' + target)
      _objs_raw = [get_variable('crt0_hosted' + target)]
    else
      _objs_raw = []
    endif

    t1 = 'test-hello-raw'
    t1_src = 'test-hello.c'
    test(t1 + target,
         executable(t1 + target, t1_src,
		    c_args: printf_compile_args_d + _c_args,
		    link_args: printf_link_args_d + _link_raw_args,
		    objects: _objs_raw,
		    link_depends:  _link_raw_depends,
		    include_directories: inc),
         depends: bios_bin,
         suite: 'test',
         should_fail: t1 in tests_should_fail,
         args: test_args.get(t1, []),
         env: test_env,
         timeout: 90,
        )

    if no_flash_tests.contains(t1)
      # Build raw no-flash using hosted crt0 noflash variant and raw libs
      _link_raw_args_no_flash = target_c_args + _lib_raw_files + get_variable('test_link_args_noflash' + target, test_link_args_noflash)
      _link_raw_depends_no_flash = get_variable('test_link_depends_noflash' + target, test_link_depends_noflash) + _libs_raw
      _objs_raw_noflash = [get_variable('crt0_noflash_hosted' + target)]
      test(t1 + '-no-flash' + target,
           executable(t1 + '-no-flash' + target, t1_src,
                      c_args: printf_compile_args_d + _c_args,
                      link_args: printf_compile_args_d + _link_raw_args_no_flash,
                      objects: _objs_raw_noflash,
                      link_depends:  _link_raw_depends_no_flash,
                      include_directories: inc),
           depends: bios_bin,
           suite: 'test-no-flash',
           should_fail: t1 in tests_should_fail,
           args: test_args.get(t1, []),
           env: test_env,
           timeout: 90,
          )
    endif
  endif

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
    if no_flash_tests.contains(t1)
      test(t1 + '-no-flash' + target,
           executable(t1 + '-no-flash' + target, t1_src,
                      cpp_args: _cpp_args,
                      link_args: _cpp_args + _link_args_no_flash,
                      objects: _objs_no_flash,
                      link_depends:  _link_depends_no_flash,
                      include_directories: inc),
           depends: bios_bin,
           suite: 'test-no-flash',
           env: test_env)
    endif
  endif

  foreach t1 : tests
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']
    test_file_name_arg_no_flash=['-DTEST_FILE_NAME="' + t1 + '_no_flash_' + target + '.txt"']

    lock_valid_src = []
    if t1 not in test_skip_lockvalid
      lock_valid_src = ['lock-valid.c']
    endif

    test(t1 + target,
	 executable(t1 + target, [t1_src] + lock_valid_src + test_extra_src.get(t1, []),
		    c_args: printf_compile_args_d + test_file_name_arg + test_extra_c_args.get(t1, []) + _c_args,
		    link_args: printf_link_args_d + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 60,
         suite: 'test',
         should_fail: t1 in tests_should_fail,
         args: test_args.get(t1, []),
	 env: test_env)

    if no_flash_tests.contains(t1)
      test(t1 + '-no-flash' + target,
           executable(t1 + '-no-flash' + target, [t1_src] + lock_valid_src + test_extra_src.get(t1, []),
                    c_args: printf_compile_args_d + test_file_name_arg_no_flash + test_extra_c_args.get(t1 + '-no-flash', []) + _c_args + ['-DINIT_TLS'],
                    link_args: printf_link_args_d + _link_args_no_flash,
                    objects: _objs_no_flash,
                    link_depends:  _link_depends_no_flash,
                    include_directories: inc),
           depends: bios_bin,
           suite: 'test-no-flash',
           should_fail: t1 in tests_should_fail,
           args: test_args.get(t1, []),
           env: test_env)
    endif
  endforeach

endforeach

if enable_native_tests

  native_lib_m = cc.find_library('m', required: false)

  native_lib = static_library('native-lib',
                              ['native-locks.c'],
                              c_args: native_c_args)

  if have_cplusplus
    test('test-cplusplus-native',
         executable('test-cplusplus-native', 'test-cplusplus.cpp',
                    cpp_args: native_cpp_args,
		    link_args: native_cpp_args),
         suite: 'test',
         env: test_env
        )
  endif

  foreach t1 : tests
    if t1 in test_skip_native
      continue
    endif

    t1_src = t1 + '.c'
    t1_name = t1 + '-native'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1_name + '.txt"']

    test(t1_name,
	 executable(t1_name, [t1_src] + test_extra_src.get(t1, []),
		    c_args: native_c_args + test_file_name_arg,
		    link_args: native_c_args),
         suite: 'test',
         should_fail: t1 in tests_should_fail,
         args: test_args.get(t1, []),
         env: test_env
        )
  endforeach
endif

subdir('libc-testsuite')
subdir('test-ctype')
subdir('test-iconv')
subdir('test-posix')
subdir('test-math')
subdir('test-string')
subdir('test-monetary')
subdir('test-stdio')
subdir('test-stdlib')
subdir('testsuite')

if has_arm_semihost and oslib_test_variant == 'semihost'
  subdir('semihost')
endif

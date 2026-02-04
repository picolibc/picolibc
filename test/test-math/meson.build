#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2025 Keith Packard
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

tests_real_trig = [
  'test-acos',
  'test-acosh',
  'test-asin',
  'test-asinh',
  'test-atan',
  'test-atan2',
  'test-atanh',
  'test-cos',
  'test-cosh',
  'test-sin',
  'test-sincos-cos',
  'test-sincos-sin',
  'test-sinh',
  'test-tan',
  'test-tanh'
]

tests_real_exp = [
  'test-exp',
  'test-exp10',
  'test-exp2',
  'test-expm1',
  'test-log',
  'test-log10',
  'test-log1p',
  'test-log2',
  'test-pow'
]

tests_real_other = [
  'test-cbrt',
  'test-erf',
  'test-erfc',
  'test-hypot',
  'test-j0',
  'test-j1',
  'test-jn',
  'test-lgamma',
  'test-sqrt',
  'test-tgamma'
]

tests_complex_trig = [
  'test-cacos',
  'test-cacosh',
  'test-casin',
  'test-casinh',
  'test-catan',
  'test-catanh',
  'test-ccos',
  'test-ccosh',
  'test-csin',
  'test-csinh',
  'test-ctan',
  'test-ctanh'
]

tests_complex_exp = [
  'test-cexp',
  'test-clog',
  'test-cpow'
]

tests_complex_other = [
  'test-csqrt'
]

tests = tests_real_trig + tests_real_exp + tests_real_other
tests += tests_complex_trig + tests_complex_exp + tests_complex_other

functions = [ 'acos', 'acosh', 'asin', 'asinh', 'atan', 'atanh',
              'cbrt', 'cos', 'cosh', 'erf', 'erfc', 'exp', 'exp10', 'exp2',
              'expm1', 'j0', 'j1', 'lgamma', 'log', 'log10', 'log1p', 'log2',
              'sin', 'sincos_cos', 'sincos_sin', 'sinh', 'sqrt', 'tan', 'tanh', 'tgamma', 'y0', 'y1',
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
  foreach t1 : tests
    t1_src = t1 + '.c'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1 + target + '.txt"']

    test(t1 + target,
	 executable(t1 + target, [t1_src],
		    c_args: printf_compile_args_d + test_file_name_arg + _c_args,
		    link_args: printf_link_args_d + _link_args,
		    objects: _objs,
		    link_depends:  _link_depends,
		    include_directories: inc),
         depends: bios_bin,
	 timeout: 60,
         suite: 'test-math',
	 env: test_env)
  endforeach

  if enable_exhaustive_math_tests
    foreach f : functions
      test_name = 'test-math-compare-' + f
      test_function_name_arg=['-DTEST_FUNC=' + f]
      test(test_name,
           executable(test_name, 'test-math-compare.c',
                      c_args: printf_compile_args_d + test_function_name_arg + _c_args,
                      link_args: printf_link_args_d + _link_args,
                      objects: _objs,
                      link_depends: _link_depends,
                      include_directories: inc),
           depends: bios_bin,
	   timeout: 7200,
           suite: 'test-math-compare',
	   env: test_env)
    endforeach
  endif

endforeach

if enable_native_math_tests

  native_lib_m = cc.find_library('m', required: false)

  foreach t1 : tests
    t1_src = t1 + '.c'
    t1_name = t1 + '-native'
    test_file_name_arg=['-DTEST_FILE_NAME="' + t1_name + '.txt"']

    test(t1_name,
	 executable(t1_name, t1_src,
		    c_args: native_c_args + test_file_name_arg + ctype_c_args,
		    link_args: native_c_args,
                    dependencies: native_lib_m),
         suite: 'test-math',
         env: test_env
        )
  endforeach

  if enable_exhaustive_math_tests
    foreach f : functions
      test_name = 'test-math-compare-' + f + '-native'
      test_function_name_arg=['-DTEST_FUNC=' + f]

      test(test_name,
	   executable(test_name, 'test-math-compare.c',
		      c_args: native_c_args + test_function_name_arg + ctype_c_args,
		      link_args: native_c_args,
                      dependencies: native_lib_m),
           suite: 'test-math-compare',
           env: test_env,
           timeout: 7200,
          )
    endforeach
  endif

endif

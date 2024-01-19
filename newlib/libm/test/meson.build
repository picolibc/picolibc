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

# Some arrays have more members than what can be supported on some targets.
# Split the arrays into chunks of ~4k, and compile a separate test for each
# chunk. Meson 0.58 has range(), but 0.53 is supported, so make an array
# of numbers to iterate over.

if get_option('split-large-tests')
  test_parts = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '10', '11', '12', '13', '14', '15', '16',
  ]
else
  test_parts = ['-1']
endif

math_test_src = [
  'acosf_vec.c',
  'acoshf_vec.c',
  'acosh_vec.c',
  'acos_vec.c',
  'asinf_vec.c',
  'asinhf_vec.c',
  'asinh_vec.c',
  'asin_vec.c',
  'atan2f_vec.c',
  'atan2_vec.c',
  'atanf_vec.c',
  'atanhf_vec.c',
  'atanh_vec.c',
  'atan_vec.c',
  'ceilf_vec.c',
  'ceil_vec.c',
  'copysign_vec.c',
  'copysignf_vec.c',
  'convert.c',
  'conv_vec.c',
  'cosf_vec.c',
  'coshf_vec.c',
  'cosh_vec.c',
  'cos_vec.c',
  'dvec.c',
  'erfcf_vec.c',
  'erfc_vec.c',
  'erff_vec.c',
  'erf_vec.c',
  'expf_vec.c',
  'exp_vec.c',
  'fabsf_vec.c',
  'fabs_vec.c',
  'floorf_vec.c',
  'floor_vec.c',
  'fmodf_vec.c',
  'fmod_vec.c',
  'gammaf_vec.c',
  'gamma_vec.c',
  'hypotf_vec.c',
  'hypot_vec.c',
  'iconv_vec.c',
  'issignaling_vec.c',
  'j0f_vec.c',
  'j0_vec.c',
  'j1f_vec.c',
  'j1_vec.c',
  'jnf_vec.c',
  'jn_vec.c',
  'log10f_vec.c',
  'log10_vec.c',
  'log1pf_vec.c',
  'log1p_vec.c',
  'log2f_vec.c',
  'log2_vec.c',
  'logf_vec.c',
  'log_vec.c',
  'math2.c',
  'math.c',
  'modf_vec.c',
  'modff_vec.c',
  'pow_vec.c',
  'powf_vec.c',
  'scalb_vec.c',
  'scalbn_vec.c',
  'sinf_vec.c',
  'sinhf_vec.c',
  'sinh_vec.c',
  'sin_vec.c',
  'sprint_ivec.c',
  'sprint_vec.c',
  'sqrtf_vec.c',
  'sqrt_vec.c',
  'string.c',
  'tanf_vec.c',
  'tanhf_vec.c',
  'tanh_vec.c',
  'tan_vec.c',
  'test.c',
  'test_is.c',
  'trunc_vec.c',
  'truncf_vec.c',
  'y0f_vec.c',
  'y0_vec.c',
  'y1f_vec.c',
  'y1_vec.c',
  'ynf_vec.c',
  'yn_vec.c',
]

if has_ieeefp_funcs
  math_test_src += [
    'test_ieee.c',
  ]
endif

#  'dcvt.c',

skip_math_test = meson.get_cross_property('skip_math_test', [])

foreach target : targets

  if target in skip_math_test
    continue
  endif

  value = get_variable('target_' + target)

  libs = [get_variable('lib_c' + target)]
  if is_variable('lib_semihost' + target)
    libs += [get_variable('lib_semihost' + target)]
  endif
  if is_variable('crt0_hosted' + target)
    objs = [get_variable('crt0_hosted'+ target)]
  else
    objs = []
  endif

  _c_args = value[1] + get_variable('test_c_args_' + target, test_c_args)
  _link_args = value[1] + get_variable('test_link_args_' + target, test_link_args)
  _link_depends = get_variable('test_link_depends_' + target, test_link_depends)

  _c_args = double_printf_compile_args + _c_args
  _link_args = double_printf_link_args + _link_args

  foreach test_part : test_parts
    if test_part == '-1'
      test_part_name = ''
    else
      test_part_name = '_' + test_part
    endif
    if target == ''
      test_name = 'math_test' + test_part_name
    else
      test_name = 'math_test' + test_part_name + '_' + target
    endif
    test(test_name,
         executable(test_name, math_test_src,
		    c_args: _c_args +  ['-DTEST_PART=' + test_part],
		    objects: objs,
		    link_with: libs,
		    link_args: _link_args,
		    link_depends: _link_depends,
		    include_directories: inc),
         depends: bios_bin,
         env: test_env)
  endforeach
endforeach

if enable_native_tests

  native_lib_m = cc.find_library('m', required: false)

  if native_lib_m.found()
    defines = ['-DINCLUDE_GENERATE', '-DTEST_PART=-1']
    test('math_test_native',
	 executable('math_test_native', math_test_src,
		    c_args: native_c_args + defines,
		    link_args: native_c_args,
		    dependencies: native_lib_m))
  endif
endif

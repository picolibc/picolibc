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
srcs_math = [
    'e_acos.c',
    'e_acosh.c',
    'e_asin.c',
    'e_atan2.c',
    'e_atanh.c',
    'e_cosh.c',
    'e_exp.c',
    'ef_acos.c',
    'ef_acosh.c',
    'ef_asin.c',
    'ef_atan2.c',
    'ef_atanh.c',
    'ef_cosh.c',
    'ef_exp.c',
    'ef_fmod.c',
    'ef_hypot.c',
    'ef_j0.c',
    'ef_j1.c',
    'ef_jn.c',
    'ef_lgamma.c',
    'ef_log10.c',
    'ef_log.c',
    'e_fmod.c',
    'ef_pow.c',
    'ef_remainder.c',
    'ef_rem_pio2.c',
    'ef_scalb.c',
    'ef_sinh.c',
    'ef_sqrt.c',
    'ef_tgamma.c',
    'e_hypot.c',
    'e_j0.c',
    'e_j1.c',
    'e_jn.c',
    'e_lgamma.c',
    'e_log10.c',
    'e_log.c',
    'e_pow.c',
    'e_remainder.c',
    'e_rem_pio2.c',
    'erf_lgamma.c',
    'er_lgamma.c',
    'e_scalb.c',
    'e_sinh.c',
    'e_sqrt.c',
    'e_tgamma.c',
    'k_cos.c',
    'kf_cos.c',
    'kf_rem_pio2.c',
    'kf_sin.c',
    'kf_tan.c',
    'k_rem_pio2.c',
    'k_sin.c',
    'k_tan.c',
    's_asinh.c',
    's_atan.c',
    's_ceil.c',
    's_cos.c',
    's_erf.c',
    's_fabs.c',
    'sf_asinh.c',
    'sf_atan.c',
    'sf_ceil.c',
    'sf_cos.c',
    'sf_erf.c',
    'sf_fabs.c',
    'sf_floor.c',
    'sf_frexp.c',
    'sf_ldexp.c',
    's_floor.c',
    's_frexp.c',
    'sf_signif.c',
    'sf_sin.c',
    'sf_tan.c',
    'sf_tanh.c',
    's_ldexp.c',
    's_signif.c',
    's_sin.c',
    's_tan.c',
    's_tanh.c',
    'w_acos.c',
    'w_acosh.c',
    'w_asin.c',
    'w_atan2.c',
    'w_atanh.c',
    'w_cosh.c',
    'w_drem.c',
    'w_exp.c',
    'wf_acos.c',
    'wf_acosh.c',
    'wf_asin.c',
    'wf_atan2.c',
    'wf_atanh.c',
    'wf_cosh.c',
    'wf_drem.c',
    'wf_exp.c',
    'wf_fmod.c',
    'wf_gamma.c',
    'wf_hypot.c',
    'wf_j0.c',
    'wf_j1.c',
    'wf_jn.c',
    'wf_lgamma.c',
    'wf_log10.c',
    'wf_log2.c',
    'wf_log.c',
    'w_fmod.c',
    'wf_pow.c',
    'wf_remainder.c',
    'wf_scalb.c',
    'wf_sincos.c',
    'wf_sinh.c',
    'wf_sqrt.c',
    'wf_tgamma.c',
    'w_gamma.c',
    'w_hypot.c',
    'w_j0.c',
    'w_j1.c',
    'w_jn.c',
    'w_lgamma.c',
    'w_log10.c',
    'w_log.c',
    'w_pow.c',
    'w_remainder.c',
    'wrf_lgamma.c',
    'wr_lgamma.c',
    'w_scalb.c',
    'w_sincos.c',
    'w_sinh.c',
    'w_sqrt.c',
    'w_tgamma.c',
]

l_srcs_math = [
    'el_hypot.c',
  ]

if have_long_double
  srcs_math += l_srcs_math
endif

c_args_exp2 = []
if meson.get_compiler('c').has_argument('-fno-builtin-exp2')
  c_args_exp2 += ['-fno-builtin-exp2', '-fno-builtin-exp2f']
  message('Set c_args_exp2')
endif

srcs_math_use = []
foreach file : srcs_math
  s_file = file.split('.')[0] + '.S'
  if file in srcs_libm_machine
    message('libm/math/' + file + ': machine overrides generic')
  elif s_file in srcs_libm_machine
    message('libm/math/' + s_file + ': machine overrides generic')
  else
    srcs_math_use += file
  endif
endforeach

math_inc = [ inc, include_directories('../common') ]

foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_math_exp2' + target,
		     static_library('math_exp2' + target,
				    ['w_exp2.c', 'wf_exp2.c'],
				    pic: false,
				    include_directories: math_inc,
				    c_args: value[1] + c_args_exp2)
		    )
	lib_math_exp2_objs = get_variable('lib_math_exp2' + target).extract_all_objects(recursive:true)
	set_variable('lib_math' + target,
		static_library('math' + target,
			       srcs_math_use,
			       objects: lib_math_exp2_objs,
			       pic: false,
			       include_directories: math_inc,
			       c_args: value[1]))
endforeach

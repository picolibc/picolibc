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
srcs_mathfp = [
    'e_acosh.c',
    'e_atanh.c',
    'ef_acosh.c',
    'ef_atanh.c',
    'ef_hypot.c',
    'ef_j0.c',
    'ef_j1.c',
    'ef_remainder.c',
    'ef_scalb.c',
    'e_hypot.c',
    'e_j0.c',
    'e_j1.c',
    'e_remainder.c',
    'erf_gamma.c',
    'erf_lgamma.c',
    'er_gamma.c',
    'er_lgamma.c',
    'e_scalb.c',
    's_acos.c',
    's_asin.c',
    's_asine.c',
    's_asinh.c',
    's_atan2.c',
    's_atan.c',
    's_atangent.c',
    's_ceil.c',
    's_cos.c',
    's_cosh.c',
    's_erf.c',
    's_exp2.c',
    's_exp.c',
    's_fabs.c',
    'sf_acos.c',
    'sf_asin.c',
    'sf_asine.c',
    'sf_asinh.c',
    'sf_atan2.c',
    'sf_atan.c',
    'sf_atangent.c',
    'sf_ceil.c',
    'sf_cos.c',
    'sf_cosh.c',
    'sf_erf.c',
    'sf_exp2.c',
    'sf_exp.c',
    'sf_fabs.c',
    'sf_floor.c',
    'sf_fmod.c',
    'sf_frexp.c',
    'sf_ispos.c',
    'sf_ldexp.c',
    'sf_log10.c',
    'sf_logarithm.c',
    'sf_log.c',
    's_floor.c',
    's_fmod.c',
    'sf_numtest.c',
    'sf_pow.c',
    's_frexp.c',
    'sf_signif.c',
    'sf_sin.c',
    'sf_sincos.c',
    'sf_sine.c',
    'sf_sineh.c',
    'sf_sinh.c',
    'sf_sqrt.c',
    'sf_tan.c',
    'sf_tanh.c',
    'sf_tgamma.c',
    's_ispos.c',
    's_ldexp.c',
    's_log10.c',
    's_logarithm.c',
    's_log.c',
    's_mathcnst.c',
    's_numtest.c',
    's_pow.c',
    's_signif.c',
    's_sin.c',
    's_sincos.c',
    's_sine.c',
    's_sineh.c',
    's_sinf.c',
    's_sinh.c',
    's_sqrt.c',
    's_tan.c',
    's_tanh.c',
    's_tgamma.c',
    'w_drem.c',
    'wf_drem.c',
    'wf_jn.c',
    'w_jn.c',
]
hdrs_mathfp = [
    'zmath.h',
]
foreach target : targets
	value = get_variable('target_' + target)
	set_variable('lib_mathfp' + target,
		static_library('mathfp' + target,
			srcs_mathfp,
			pic: false,
			include_directories: [ inc, include_directories('../common') ],
			c_args: value[1]))
endforeach

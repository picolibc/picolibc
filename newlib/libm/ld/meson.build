#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2022 Keith Packard
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

srcs_ld_use = []

if have_long_double

srcs_ld_arch = [
    'e_acoshl.c',
    'e_atanhl.c',
    'e_coshl.c',
    'e_expl.c',
    'e_fmodl.c',
    'e_hypotl.c',
    'e_lgammal_r.c',
    'e_log10l.c',
    'e_log2l.c',
    'e_logl.c',
    'e_powl.c',
    'e_sinhl.c',
    'e_tgammal.c',
    'invtrig.c',
    'k_cosl.c',
    'k_sinl.c',
    'k_tanl.c',
    's_asinhl.c',
    's_ceill.c',
    's_copysignl.c',
    's_erfl.c',
    's_exp2l.c',
    's_expm1l.c',
    's_floorl.c',
    's_fpclassifyl.c',
    's_getpayloadl.c',
    's_log1pl.c',
    's_modfl.c',
    's_nanl.c',
    's_nextafterl.c',
    's_nexttoward.c',
    's_nexttowardf.c',
    's_remquol.c',
    's_significandl.c',
    's_tanhl.c',
    's_truncl.c',
   ]

srcs_ld_common = [
  'e_acosl.c',
  'e_asinl.c',
  'e_atan2l.c',
  'e_lgammal.c',
  'e_remainderl.c',
  'e_sqrtl.c',
  'k_rem_pio2.c',
  'math_errl_check_oflowl.c',
  'math_errl_check_uflowl.c',
  'math_errl_divzerol.c',
  'math_errl_invalidl.c',
  'math_errl_oflowl.c',
  'math_errl_uflowl.c',
  'math_errl_with_errnol.c',
  'polevll.c',
  's_atanl.c',
  's_ilogbl.c',
  's_cbrtl.c',
  's_cosl.c',
  's_fabsl.c',
  's_fdiml.c',
  's_finitel.c',
  's_fmal.c',
  's_fmaxl.c',
  's_fminl.c',
  's_frexpl.c',
  's_isinfl.c',
  's_isnanl.c',
  's_issignalingl.c',
  's_llrintl.c',
  's_llroundl.c',
  's_logbl.c',
  's_lrintl.c',
  's_lroundl.c',
  's_rintl.c',
  's_roundl.c',
  's_scalbl.c',
  's_scalbln.c',
  's_scalbnl.c',
  's_sincosl.c',
  's_sinl.c',
  's_tanl.c',
]

srcs_ld = srcs_ld_arch + srcs_ld_common

foreach file : srcs_ld
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_libm_machine
    message('libm/ld/' + file + ': machine overrides generic')
  elif s_file in srcs_libm_machine
    message('libm/ld/' + s_file + ': machine overrides generic')
  else
    srcs_ld_use += file
  endif
endforeach

endif

src_libm_ld = files(srcs_ld_use)

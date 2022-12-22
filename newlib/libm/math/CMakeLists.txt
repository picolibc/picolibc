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
picolibc_sources(
  k_cos.c
  k_rem_pio2.c
  k_sin.c
  k_tan.c
  kf_cos.c
  kf_rem_pio2.c
  kf_sin.c
  kf_tan.c
  s_acos.c
  s_acosh.c
  s_asin.c
  s_asinh.c
  s_atan.c
  s_atan2.c
  s_atanh.c
  s_ceil.c
  s_cos.c
  s_cosh.c
  s_drem.c
  s_erf.c
  s_exp.c
  s_exp2.c
  s_fabs.c
  s_floor.c
  s_fmod.c
  s_frexp.c
  s_hypot.c
  s_j0.c
  s_j1.c
  s_jn.c
  s_lgamma.c
  s_log.c
  s_log10.c
  s_pow.c
  s_rem_pio2.c
  s_remainder.c
  s_scalb.c
  s_signif.c
  s_sin.c
  s_sincos.c
  s_sinh.c
  s_sqrt.c
  s_tan.c
  s_tanh.c
  s_tgamma.c
  sf_acos.c
  sf_acosh.c
  sf_asin.c
  sf_asinh.c
  sf_atan.c
  sf_atan2.c
  sf_atanh.c
  sf_ceil.c
  sf_cos.c
  sf_cosh.c
  sf_drem.c
  sf_erf.c
  sf_exp.c
  sf_exp2.c
  sf_fabs.c
  sf_floor.c
  sf_fmod.c
  sf_frexp.c
  sf_hypot.c
  sf_j0.c
  sf_j1.c
  sf_jn.c
  sf_lgamma.c
  sf_log.c
  sf_log10.c
  sf_log2.c
  sf_pow.c
  sf_rem_pio2.c
  sf_remainder.c
  sf_scalb.c
  sf_signif.c
  sf_sin.c
  sf_sincos.c
  sf_sinh.c
  sf_sqrt.c
  sf_tan.c
  sf_tanh.c
  sf_tgamma.c
  sr_lgamma.c
  srf_lgamma.c
)

if(_HAVE_LONG_DOUBLE)
  picolibc_sources(
    sl_hypot.c
    )
endif()

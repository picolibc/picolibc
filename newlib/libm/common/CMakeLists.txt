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
target_include_directories(c PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

picolibc_sources(
  signgam.c
  s_finite.c
  s_copysign.c
  s_modf.c
  s_scalbn.c
  s_cbrt.c
  s_exp10.c
  s_expm1.c
  s_ilogb.c
  s_infinity.c
  s_iseqsig.c
  s_isinf.c
  s_isinfd.c
  s_isnan.c
  s_isnand.c
  s_issignaling.c
  s_log1p.c
  s_nan.c
  s_nextafter.c
  s_pow10.c
  s_rint.c
  s_logb.c
  s_log2.c
  s_fdim.c
  s_fma.c
  s_fmax.c
  s_fmin.c
  s_fpclassify.c
  s_getpayload.c
  s_lrint.c
  s_llrint.c
  s_lround.c
  s_llround.c
  s_nearbyint.c
  s_remquo.c
  s_round.c
  s_scalbln.c
  s_signbit.c
  s_trunc.c
  exp.c
  exp2.c
  exp_data.c
  math_err_with_errno.c
  math_err_uflow.c
  math_err_oflow.c
  math_err_divzero.c
  math_err_invalid.c
  math_err_may_uflow.c
  math_err_check_uflow.c
  math_err_check_oflow.c
  math_inexact.c
  math_inexactf.c
  log.c
  log_data.c
  log2.c
  log2_data.c
  pow.c
  pow_log_data.c
  sf_finite.c
  sf_copysign.c
  sf_modf.c
  sf_scalbn.c
  sf_cbrt.c
  sf_exp10.c
  sf_expm1.c
  sf_getpayload.c
  sf_ilogb.c
  sf_infinity.c
  sf_iseqsig.c
  sf_isinf.c
  sf_isinff.c
  sf_isnan.c
  sf_isnanf.c
  sf_issignaling.c
  sf_log1p.c
  sf_nan.c
  sf_nextafter.c
  sf_pow10.c
  sf_rint.c
  sf_logb.c
  sf_fdim.c
  sf_fma.c
  sf_fmax.c
  sf_fmin.c
  sf_fpclassify.c
  sf_lrint.c
  sf_llrint.c
  sf_lround.c
  sf_llround.c
  sf_nearbyint.c
  sf_remquo.c
  sf_round.c
  sf_scalbln.c
  sf_trunc.c
  math_errf_with_errnof.c
  math_errf_uflowf.c
  math_errf_may_uflowf.c
  math_errf_oflowf.c
  math_errf_divzerof.c
  math_errf_invalidf.c
  math_errf_check_oflowf.c
  math_errf_check_uflowf.c
  )

if(NOT __OBSOLETE_MATH_FLOAT)
  picolibc_sources(
    cosf.c
    sf_exp.c
    sf_exp2.c
    sf_exp2_data.c
    sf_log.c
    sf_log2.c
    sf_log2_data.c
    sf_log_data.c
    sf_pow.c
    sf_pow_log2_data.c
    sincosf.c
    sincosf_data.c
    sinf.c
    )
endif()

if(NOT __OBSOLETE_MATH_DOUBLE)
  picolibc_sources(
    exp.c
    exp2.c
    exp_data.c
    log.c
    log2.c
    log2_data.c
    log_data.c
    pow.c
    pow_log_data.c
    s_log2.c
    )
endif()  

if(_HAVE_LONG_DOUBLE)
  picolibc_sources(
    copysignl.c
    frexpl.c
    isinfl.c
    isnanl.c
    math_errl_invalidl.c
    math_errl_with_errnol.c
    nanl.c
    sl_finite.c
    sl_iseqsig.c
    sl_issignaling.c
    sqrtl.c
    )
  if (_LDBL_EQ_DBL)
    picolibc_sources(
      atanl.c
      cosl.c
      sinl.c
      tanl.c
      tanhl.c
      frexpl.c
      modfl.c
      ceill.c
      fabsl.c
      floorl.c
      log1pl.c
      expm1l.c
      acosl.c
      asinl.c
      atan2l.c
      coshl.c
      sinhl.c
      expl.c
      ldexpl.c
      logl.c
      log10l.c
      powl.c
      fmodl.c
      ilogbl.c
      asinhl.c
      cbrtl.c
      nextafterl.c
      rintl.c
      scalbnl.c
      exp2l.c
      scalblnl.c
      tgammal.c
      nearbyintl.c
      lrintl.c
      llrintl.c
      roundl.c
      lroundl.c
      llroundl.c
      truncl.c
      remquol.c
      fdiml.c
      fmaxl.c
      fminl.c
      fmal.c
      acoshl.c
      atanhl.c
      remainderl.c
      lgammal.c
      erfl.c
      erfcl.c
      logbl.c
      nexttowardf.c
      nexttoward.c
      nexttowardl.c
      log2l.c
      dreml.c
      exp10l.c
      pow10l.c
      sincosl.c
      )
  endif()
endif()

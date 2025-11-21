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

set(tests_real_trig
  test-acos
  test-acosh
  test-asin
  test-asinh
  test-atan
  test-atan2
  test-atanh
  test-cos
  test-cosh
  test-sin
  test-sinh
  test-tan
  test-tanh
)

set(tests_real_exp
  test-exp
  test-exp10
  test-exp2
  test-expm1
  test-log
  test-log10
  test-log1p
  test-log2
  test-pow
)

set(tests_real_other
  test-cbrt
  test-erf
  test-erfc
  test-hypot
  test-j0
  test-j1
  test-jn
  test-lgamma
  test-sqrt
  test-tgamma
  )

set(tests_complex_trig
  test-cacos
  test-cacosh
  test-casin
  test-casinh
  test-catan
  test-catanh
  test-ccos
  test-ccosh
  test-csin
  test-csinh
  test-ctan
  test-ctanh
)

set(tests_complex_exp
  test-cexp
  test-clog
  test-cpow
)

set(tests_complex_other
  test-csqrt
)

set(tests
  ${tests_real_trig}
  ${tests_real_exp}
  ${tests_real_other}
  ${tests_complex_trig}
  ${tests_complex_exp}
  ${tests_complex_other}
  )

foreach(test IN LISTS tests)
  picolibc_test(${test})
endforeach()


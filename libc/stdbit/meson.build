#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2026 Keith Packard
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

srcs_stdbit = [
  'stdc_bit_ceil_8.c',
  'stdc_bit_ceil_16.c',
  'stdc_bit_ceil_32.c',
  'stdc_bit_ceil_64.c',
  'stdc_bit_ceil_128.c',
  'stdc_bit_floor_8.c',
  'stdc_bit_floor_16.c',
  'stdc_bit_floor_32.c',
  'stdc_bit_floor_64.c',
  'stdc_bit_floor_128.c',
  'stdc_bit_width_8.c',
  'stdc_bit_width_16.c',
  'stdc_bit_width_32.c',
  'stdc_bit_width_64.c',
  'stdc_bit_width_128.c',
  'stdc_count_ones_8.c',
  'stdc_count_ones_16.c',
  'stdc_count_ones_32.c',
  'stdc_count_ones_64.c',
  'stdc_count_ones_128.c',
  'stdc_count_zeros_8.c',
  'stdc_count_zeros_16.c',
  'stdc_count_zeros_32.c',
  'stdc_count_zeros_64.c',
  'stdc_count_zeros_128.c',
  'stdc_first_leading_one_8.c',
  'stdc_first_leading_one_16.c',
  'stdc_first_leading_one_32.c',
  'stdc_first_leading_one_64.c',
  'stdc_first_leading_one_128.c',
  'stdc_first_leading_zero_8.c',
  'stdc_first_leading_zero_16.c',
  'stdc_first_leading_zero_32.c',
  'stdc_first_leading_zero_64.c',
  'stdc_first_leading_zero_128.c',
  'stdc_first_trailing_one_8.c',
  'stdc_first_trailing_one_16.c',
  'stdc_first_trailing_one_32.c',
  'stdc_first_trailing_one_64.c',
  'stdc_first_trailing_one_128.c',
  'stdc_first_trailing_zero_8.c',
  'stdc_first_trailing_zero_16.c',
  'stdc_first_trailing_zero_32.c',
  'stdc_first_trailing_zero_64.c',
  'stdc_first_trailing_zero_128.c',
  'stdc_has_single_bit_8.c',
  'stdc_has_single_bit_16.c',
  'stdc_has_single_bit_32.c',
  'stdc_has_single_bit_64.c',
  'stdc_has_single_bit_128.c',
  'stdc_leading_ones_8.c',
  'stdc_leading_ones_16.c',
  'stdc_leading_ones_32.c',
  'stdc_leading_ones_64.c',
  'stdc_leading_ones_128.c',
  'stdc_leading_zeros_8.c',
  'stdc_leading_zeros_16.c',
  'stdc_leading_zeros_32.c',
  'stdc_leading_zeros_64.c',
  'stdc_leading_zeros_128.c',
  'stdc_trailing_ones_8.c',
  'stdc_trailing_ones_16.c',
  'stdc_trailing_ones_32.c',
  'stdc_trailing_ones_64.c',
  'stdc_trailing_ones_128.c',
  'stdc_trailing_zeros_8.c',
  'stdc_trailing_zeros_16.c',
  'stdc_trailing_zeros_32.c',
  'stdc_trailing_zeros_64.c',
  'stdc_trailing_zeros_128.c',
  ]

srcs_stdbit_use = []

foreach file : srcs_stdbit
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/stdbit/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/stdbit/' + s_file + ': machine overrides generic')
  else
    srcs_stdbit_use += file
  endif
endforeach

src_stdbit = files(srcs_stdbit_use)

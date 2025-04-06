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
srcs_ubsan = [
  'ubsan_error.c',
  'ubsan_handle_add_overflow.c',
  'ubsan_handle_alignment_assumption.c',
  'ubsan_handle_builtin_unreachable.c',
  'ubsan_handle_divrem_overflow.c',
  'ubsan_handle_float_cast_overflow.c',
  'ubsan_handle_function_type_mismatch.c',
  'ubsan_handle_implicit_conversion.c',
  'ubsan_handle_invalid_builtin.c',
  'ubsan_handle_invalid_objc_cast.c',
  'ubsan_handle_load_invalid_value.c',
  'ubsan_handle_missing_return.c',
  'ubsan_handle_mul_overflow.c',
  'ubsan_handle_negate_overflow.c',
  'ubsan_handle_nonnull_arg.c',
  'ubsan_handle_nonnull_return.c',
  'ubsan_handle_nonnull_return_v1.c',
  'ubsan_handle_nullability_arg.c',
  'ubsan_handle_nullability_return.c',
  'ubsan_handle_nullability_return_v1.c',
  'ubsan_handle_out_of_bounds.c',
  'ubsan_handle_pointer_overflow.c',
  'ubsan_handle_shift_out_of_bounds.c',
  'ubsan_handle_sub_overflow.c',
  'ubsan_handle_type_mismatch.c',
  'ubsan_handle_type_mismatch_v1.c',
  'ubsan_message.c',
  'ubsan_type_check_to_string.c',
  'ubsan_val_to_imax.c',
  'ubsan_val_to_string.c',
  'ubsan_val_to_umax.c',
  'ubsan_warning.c',
  ]

srcs_ubsan_use = []
foreach file : srcs_ubsan
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/ubsan/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/ubsan/' + s_file + ': machine overrides generic')
  else
    srcs_ubsan_use += file
  endif
endforeach

src_ubsan = files(srcs_ubsan_use)

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
srcs_ctype = [
    'categories.c',
    'ctype_.c',
    'isalnum.c',
    'isalnum_l.c',
    'isalpha.c',
    'isalpha_l.c',
    'isascii.c',
    'isascii_l.c',
    'isblank.c',
    'isblank_l.c',
    'iscntrl.c',
    'iscntrl_l.c',
    'isdigit.c',
    'isdigit_l.c',
    'isgraph.c',
    'isgraph_l.c',
    'islower.c',
    'islower_l.c',
    'isprint.c',
    'isprint_l.c',
    'ispunct.c',
    'ispunct_l.c',
    'isspace.c',
    'isspace_l.c',
    'isupper.c',
    'isupper_l.c',
    'iswalnum.c',
    'iswalnum_l.c',
    'iswalpha.c',
    'iswalpha_l.c',
    'iswblank.c',
    'iswblank_l.c',
    'iswcntrl.c',
    'iswcntrl_l.c',
    'iswctype.c',
    'iswctype_l.c',
    'iswdigit.c',
    'iswdigit_l.c',
    'iswgraph.c',
    'iswgraph_l.c',
    'iswlower.c',
    'iswlower_l.c',
    'iswprint.c',
    'iswprint_l.c',
    'iswpunct.c',
    'iswpunct_l.c',
    'iswspace.c',
    'iswspace_l.c',
    'iswupper.c',
    'iswupper_l.c',
    'iswxdigit.c',
    'iswxdigit_l.c',
    'isxdigit.c',
    'isxdigit_l.c',
    'jp2uc.c',
    'toascii.c',
    'toascii_l.c',
    'tolower.c',
    'tolower_l.c',
    'toupper.c',
    'toupper_l.c',
    'towctrans.c',
    'towctrans_l.c',
    'towlower.c',
    'towlower_l.c',
    'towupper.c',
    'towupper_l.c',
    'wctrans.c',
    'wctrans_l.c',
    'wctype.c',
    'wctype_l.c',
]
hdrs_ctype = [
    'categories.h',
    'ctype_cp.h',
    'ctype_.h',
    'ctype_iso.h',
    'jp2uc.h',
    'local.h',
]

srcs_ctype_use = []
foreach file : srcs_ctype
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/ctype/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/ctype/' + s_file + ': machine overrides generic')
  else
    srcs_ctype_use += file
  endif
endforeach

src_ctype = files(srcs_ctype_use)

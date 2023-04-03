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
subdir('iconv_data')

srcs_iconv_ccs = [
    'big5.c',
    'ccsbi.c',
    'cns11643_plane14.c',
    'cns11643_plane1.c',
    'cns11643_plane2.c',
    'cp775.c',
    'cp850.c',
    'cp852.c',
    'cp855.c',
    'cp866.c',
    'iso_8859_10.c',
    'iso_8859_11.c',
    'iso_8859_13.c',
    'iso_8859_14.c',
    'iso_8859_15.c',
    'iso_8859_1.c',
    'iso_8859_2.c',
    'iso_8859_3.c',
    'iso_8859_4.c',
    'iso_8859_5.c',
    'iso_8859_6.c',
    'iso_8859_7.c',
    'iso_8859_8.c',
    'iso_8859_9.c',
    'iso_ir_111.c',
    'jis_x0201_1976.c',
    'jis_x0208_1990.c',
    'jis_x0212_1990.c',
    'koi8_r.c',
    'koi8_ru.c',
    'koi8_u.c',
    'koi8_uni.c',
    'ksx1001.c',
    'win_1250.c',
    'win_1251.c',
    'win_1252.c',
    'win_1253.c',
    'win_1254.c',
    'win_1255.c',
    'win_1256.c',
    'win_1257.c',
    'win_1258.c',
]
hdrs_iconv_ccs = [
    'ccsbi.h',
    'ccs.h',
    'ccsnames.h',
]

newlib_iconv_ccs_dir=meson.current_source_dir()

srcs_iconv_ccs_use = []
foreach file : srcs_iconv_ccs
  s_file = fs.replace_suffix(file, '.S')
  if file in srcs_machine
    message('libc/iconv/ccs/' + file + ': machine overrides generic')
  elif s_file in srcs_machine
    message('libc/iconv/ccs/' + s_file + ': machine overrides generic')
  else
    srcs_iconv_ccs_use += file
  endif
endforeach

src_iconv_ccs = files(srcs_iconv_ccs_use)


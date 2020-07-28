#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright © 2020 Keith Packard
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

cctfiles = [
    'big5.cct',
    'cns11643_plane14.cct',
    'cns11643_plane1.cct',
    'cns11643_plane2.cct',
    'cp775.cct',
    'cp850.cct',
    'cp852.cct',
    'cp855.cct',
    'cp866.cct',
    'iso_8859_10.cct',
    'iso_8859_11.cct',
    'iso_8859_13.cct',
    'iso_8859_14.cct',
    'iso_8859_15.cct',
    'iso_8859_1.cct',
    'iso_8859_2.cct',
    'iso_8859_3.cct',
    'iso_8859_4.cct',
    'iso_8859_5.cct',
    'iso_8859_6.cct',
    'iso_8859_7.cct',
    'iso_8859_8.cct',
    'iso_8859_9.cct',
    'iso_ir_111.cct',
    'jis_x0201_1976.cct',
    'jis_x0208_1990.cct',
    'jis_x0212_1990.cct',
    'koi8_r.cct',
    'koi8_ru.cct',
    'koi8_u.cct',
    'koi8_uni.cct',
    'ksx1001.cct',
    'win_1250.cct',
    'win_1251.cct',
    'win_1252.cct',
    'win_1253.cct',
    'win_1254.cct',
    'win_1255.cct',
    'win_1256.cct',
    'win_1257.cct',
    'win_1258.cct',
]

if newlib_iconv_external_ccs
  install_data(cctfiles, install_dir: join_paths(newlib_iconv_dir, 'iconv_data'))
endif
